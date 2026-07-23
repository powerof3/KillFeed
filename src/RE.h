#pragma once

namespace RE
{
	template <class... Args>
	void DispatchStaticCall(BSFixedString a_class, BSFixedString a_fnName, BSTSmartPointer<BSScript::IStackCallbackFunctor> a_callback, Args&&... a_args)
	{
		if (auto vm = BSScript::Internal::VirtualMachine::GetSingleton()) {
			std::unique_ptr<RE::BSScript::IFunctionArguments> args{ MakeFunctionArguments(std::forward<Args>(a_args)...) };
			vm->DispatchStaticCall(a_class, a_fnName, args.get(), a_callback);
		}
	}

	template <class... Args>
	void DispatchStaticCall(BSFixedString a_class, BSFixedString a_fnName, Args&&... a_args)
	{
		BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
		DispatchStaticCall(a_class, a_fnName, callback, std::forward<Args>(a_args)...);
	}

	float       GetHeadingAngleFromPlayer(const TESObjectREFRPtr& a_victim);
	void        SetActiveTextInputMaxChars();
	std::string GetNPCName(const TESObjectREFRPtr& a_actor);

	struct FormLogger
	{
		std::string FormToStr() const
		{
			if (!form || form->IsDisabled() || form->IsDeleted()) {
				return "NULL";
			}
			auto        ref = form->AsReference();
			const auto& name = ref ? ref->GetName() : clib_util::editorID::get_editorID(form);
			if (form->IsDynamicForm()) {
				if (auto baseObj = ref ? ref->GetBaseObject() : nullptr) {
					return std::format("'{}' [0x{:X} (0x{:X}~{})]", name, form->GetFormID(), baseObj->GetFormID(), baseObj->GetFile(0) ? baseObj->GetFile(0)->fileName : "NULL");
				}
			}
			if (auto file = form->GetFile(0); file) {
				return std::format("'{}' [0x{:X}~{}]", name, form->GetLocalFormID(), file->fileName);
			}
			return std::format("'{}' [0x{:X}]", name, form->GetFormID());
		}

		const RE::TESForm* form{};
	};
}

template <>
struct fmt::formatter<RE::FormLogger>
{
	template <class ParseContext>
	constexpr auto parse(ParseContext& a_ctx)
	{
		return a_ctx.begin();
	}

	template <class FormatContext>
	auto format(const RE::FormLogger& a_logger, FormatContext& a_ctx) const
	{
		return fmt::format_to(a_ctx.out(), "{}", a_logger.FormToStr());
	}
};
