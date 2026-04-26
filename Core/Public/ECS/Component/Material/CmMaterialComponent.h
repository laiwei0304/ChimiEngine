#pragma once

#include "Render/CmMesh.h"
#include "Render/CmMaterial.h"
#include "ECS/CmComponent.h"

namespace chimi{
    template<typename T>
    class CmMaterialComponent : public CmComponent{
    public:
        void CmdMesh(CmMesh *mesh, T *material = nullptr){
            if(!mesh){
                return;
            }
            uint32_t meshIndex = mMeshList.size();
            mMeshList.push_back(mesh);

            if(mMeshMaterials.find(material) != mMeshMaterials.end()){
                mMeshMaterials[material].push_back(meshIndex);
            } else {
                mMeshMaterials.insert({ material, { meshIndex } });
            }
        }

        uint32_t GetMaterialCount() const {
            return mMeshMaterials.size();
        }

        const std::unordered_map<T*, std::vector<uint32_t>> &GetMeshMaterials() const {
            return mMeshMaterials;
        }

        CmMesh *GetMesh(uint32_t index) const {
            if(index < mMeshList.size()){
                return mMeshList[index];
            }
            return nullptr;
        }
    private:
        std::vector<CmMesh*> mMeshList;
        std::unordered_map<T*, std::vector<uint32_t>> mMeshMaterials;
    };
}