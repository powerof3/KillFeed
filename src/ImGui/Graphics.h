#pragma once

namespace ImGui
{
	struct IconTexture
	{
		IconTexture() = default;

		bool Load(ID3D11Device* a_device, std::wstring_view a_texturePath, float a_textureScale);
		void ButtonIcon(float a_posY, float a_entryHeight, const ImVec4& a_iconTint) const;

		// members
		ComPtr<ID3D11ShaderResourceView> srView{ nullptr };
		ImVec2                           size{};
	};
}
