#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define DIRECTINPUT_VERSION 0x0800
#define IMGUI_DEFINE_MATH_OPERATORS
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "RE/Skyrim.h"
#include "REX/REX.h"
#include "SKSE/SKSE.h"

#include <codecvt>
#include <wrl/client.h>

#include <DirectXMath.h>
#include <DirectXTex.h>
#include <boost/circular_buffer.hpp>
#include <boost/regex.hpp>
#include <boost/unordered/concurrent_node_map.hpp>
#include <boost/unordered/concurrent_node_set.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <freetype/freetype.h>
#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>
#include <xbyak/xbyak.h>

#include "imgui_internal.h"
#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_stdlib.h>

#include <ClibUtil/editorID.hpp>

#include <SimpleINI.h>
#undef ERROR

using namespace std::literals;
using namespace clib_util;
using namespace REX::STR::literals;
using namespace RE::literals;

using EventResult = RE::BSEventNotifyControl;

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

template <class K, class D, class H = boost::hash<K>, class KEqual = std::equal_to<K>>
using FlatMap = boost::unordered_flat_map<K, D, H, KEqual>;

template <class K, class H = boost::hash<K>, class KEqual = std::equal_to<K>>
using FlatSet = boost::unordered_flat_set<K, H, KEqual>;

template <class K, class D, class H = boost::hash<K>, class KEqual = std::equal_to<K>>
using NodeMap = boost::concurrent_node_map<K, D, H, KEqual>;

template <class K, class H = boost::hash<K>, class KEqual = std::equal_to<K>>
using NodeSet = boost::concurrent_node_set<K, H, KEqual>;

struct string_hash
{
	using is_transparent = void;  // enable heterogeneous overloads

	std::size_t operator()(std::string_view str) const
	{
		std::size_t seed = 0;
		for (auto it = str.begin(); it != str.end(); ++it) {
			boost::hash_combine(seed, std::tolower(*it));
		}
		return seed;
	}
};

struct string_cmp
{
	using is_transparent = void;  // enable heterogeneous overloads

	bool operator()(const std::string& str1, const std::string& str2) const
	{
		return REX::STR::IEQUALS(str1, str2);
	}
	bool operator()(std::string_view str1, std::string_view str2) const
	{
		return REX::STR::IEQUALS(str1, str2);
	}
};

template <class D>
using StringMap = FlatMap<std::string, D, string_hash, string_cmp>;
using StringSet = FlatSet<std::string, string_hash, string_cmp>;

namespace stl
{
	template <typename T, typename... Args>
	bool any_of(T value, Args... args)
	{
		return ((value == args) || ...);
	}

	template <class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
		auto& trampoline = REL::GetTrampoline();
		T::func = trampoline.write_call<5>(a_src, T::thunk);
	}

	template <class F, class T>
	void write_vfunc()
	{
		REL::Relocation<std::uintptr_t> vtbl{ F::VTABLE[0] };
		T::func = vtbl.write_vfunc(T::idx, T::thunk);
	}

	template <class T, std::size_t BYTES>
	void hook_function_prologue(std::uintptr_t a_src)
	{
		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_originalFuncAddr, std::size_t a_originalByteLength)
			{
				// Hook returns here. Execute the restored bytes and jump back to the original function.
				for (size_t i = 0; i < a_originalByteLength; ++i) {
					db(*reinterpret_cast<std::uint8_t*>(a_originalFuncAddr + i));
				}

				jmp(ptr[rip]);
				dq(a_originalFuncAddr + a_originalByteLength);
			}
		};

		Patch p(a_src, BYTES);
		p.ready();

		auto& trampoline = REL::GetTrampoline();
		trampoline.write_jmp<5>(a_src, T::thunk);

		auto alloc = trampoline.allocate(p.getSize());
		std::memcpy(alloc, p.getCode(), p.getSize());

		T::func = reinterpret_cast<std::uintptr_t>(alloc);
	}

	template <class T>
	T& get_value(CSimpleIniA& a_ini, T& a_value, const char* a_section, const char* a_key, const char* a_delimiter = R"(|)")
	{
		if constexpr (std::is_same_v<T, bool>) {
			a_value = a_ini.GetBoolValue(a_section, a_key, a_value);
		} else if constexpr (std::is_floating_point_v<T>) {
			a_value = static_cast<float>(a_ini.GetDoubleValue(a_section, a_key, a_value));
		} else if constexpr (std::is_enum_v<T>) {
			a_value = REX::STR::TO_NUM<T>(a_ini.GetValue(a_section, a_key, std::to_string(std::to_underlying(a_value)).c_str()));
		} else if constexpr (std::is_arithmetic_v<T>) {
			a_value = REX::STR::TO_NUM<T>(a_ini.GetValue(a_section, a_key, std::to_string(a_value).c_str()));
		} else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
			a_value = REX::STR::SPLIT(a_ini.GetValue(a_section, a_key, REX::STR::JOIN(a_value, a_delimiter).c_str()), a_delimiter);
		} else {
			a_value = a_ini.GetValue(a_section, a_key, a_value.c_str());
		}
		return a_value;
	}
}

namespace Runtime
{
	inline constexpr REL::Version SSE_1_7_99(1, 7, 99, 0);
	inline constexpr REL::Version MIN_ADDRESS_LIBRARY_V5 = SSE_1_7_99;

	[[nodiscard]] inline bool IsAtLeast1_7_99() noexcept
	{
		static bool result = REX::FModule::GetExecutingModule().GetFileVersion() >= Runtime::SSE_1_7_99;
		return result;
	}
}

#ifdef SKYRIM_AE
#	define OFFSET(se, ae) ae
#else
#	define OFFSET(se, ae) se
#endif

#define ICON_PATH(name) L"Data\\Interface\\KillFeed\\Textures\\" name ".png"

#include "Compatibility.h"
#include "RE.h"
#include "Version.h"
