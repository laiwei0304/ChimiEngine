#include "Render/CmMaterial.h"

namespace chimi{
    CmMaterialFactory CmMaterialFactory::s_MaterialFactory{};

    bool CmMaterial::HasTexture(uint32_t id) const {
        if(mTextures.find(id) != mTextures.end()){
            return true;
        }
        return false;
    }

    const TextureView *CmMaterial::GetTextureView(uint32_t id) const {
        if(HasTexture(id)){
            return &mTextures.at(id);
        }
        return nullptr;
    }

    void CmMaterial::SetTextureView(uint32_t id, CmTexture *texture, CmSampler *sampler) {
        if(HasTexture(id)){
            mTextures[id].texture = texture;
            mTextures[id].sampler = sampler;
        } else {
            mTextures[id] = { texture, sampler };
        }
        MarkResourceDirty();
    }

    void CmMaterial::UpdateTextureViewEnable(uint32_t id, bool enable) {
        if(HasTexture(id)){
            mTextures[id].bEnable = enable;
            MarkParamsDirty();
        }
    }

    void CmMaterial::UpdateTextureViewUVTranslation(uint32_t id, const glm::vec2 &uvTranslation) {
        if(HasTexture(id)){
            mTextures[id].uvTranslation = uvTranslation;
            MarkParamsDirty();
        }
    }

    void CmMaterial::UpdateTextureViewUVRotation(uint32_t id, float uvRotation) {
        if(HasTexture(id)){
            mTextures[id].uvRotation = uvRotation;
            MarkParamsDirty();
        }
    }

    void CmMaterial::UpdateTextureViewUVScale(uint32_t id, const glm::vec2 &uvScale) {
        if(HasTexture(id)){
            mTextures[id].uvScale = uvScale;
            MarkParamsDirty();
        }
    }
}
