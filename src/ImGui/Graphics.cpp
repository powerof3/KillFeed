#include "Graphics.h"

namespace ImGui
{
	bool IconTexture::Load(ID3D11Device* a_device, std::wstring_view a_texturePath, float a_textureScale)
	{
		bool result = false;

		if (srView) {
			srView.Reset();
		}

		auto image = std::make_unique<DirectX::ScratchImage>();
		auto hr = DirectX::LoadFromWICFile(a_texturePath.data(), DirectX::WIC_FLAGS_IGNORE_SRGB, nullptr, *image);
		if (SUCCEEDED(hr)) {
			auto targetWidth = static_cast<std::size_t>(image->GetMetadata().width * a_textureScale);
			auto targetHeight = static_cast<std::size_t>(image->GetMetadata().height * a_textureScale);

			auto resizedImage = std::make_unique<DirectX::ScratchImage>();
			hr = DirectX::Resize(
				image->GetImages(),
				image->GetImageCount(),
				image->GetMetadata(),
				targetWidth,
				targetHeight,
				DirectX::TEX_FILTER_TRIANGLE,
				*resizedImage);
			if (SUCCEEDED(hr)) {
				ComPtr<ID3D11Resource> pTexture{};
				hr = DirectX::CreateTexture(a_device, resizedImage->GetImages(), resizedImage->GetImageCount(), resizedImage->GetMetadata(), pTexture.GetAddressOf());
				if (SUCCEEDED(hr)) {
					D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
					srvDesc.Format = resizedImage->GetMetadata().format;
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MipLevels = 1;
					srvDesc.Texture2D.MostDetailedMip = 0;
					hr = a_device->CreateShaderResourceView(pTexture.Get(), &srvDesc, srView.GetAddressOf());
					result = SUCCEEDED(hr);
				}
				size.x = static_cast<float>(resizedImage->GetMetadata().width);
				size.y = static_cast<float>(resizedImage->GetMetadata().height);
			}
		}

		return result;
	}

	void IconTexture::ButtonIcon(float a_posY, float a_entryHeight, const ImVec4& a_iconTint) const
	{
		ImGui::SetCursorPosY(a_posY + ((a_entryHeight - size.y) * 0.5f));
		ImGui::ImageWithBg(srView.Get(), size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), a_iconTint);
	}
}
