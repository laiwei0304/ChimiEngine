#include "CmEditorLayer.h"

#include "CmEditorContext.h"
#include "CmApplication.h"
#include "ECS/CmScene.h"
#include "ECS/CmEntity.h"
#include "ECS/CmNode.h"
#include "ECS/Component/CmTransformComponent.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace chimi{
    namespace{
        void DrawNode(CmNode *node, CmEditorContext *context){
            if(!node){
                return;
            }

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
            if(!node->HasChildren()){
                flags |= ImGuiTreeNodeFlags_Leaf;
            }

            CmEntity *entity = dynamic_cast<CmEntity*>(node);
            if(entity && context->GetSelectedEntity() == entity){
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            const std::string &name = node->GetName();
            bool opened = ImGui::TreeNodeEx(node, flags, "%s", name.empty() ? "Node" : name.c_str());
            if(entity && ImGui::IsItemClicked()){
                context->SetSelectedEntity(entity);
            }
            if(opened){
                for(CmNode *child: node->GetChildren()){
                    DrawNode(child, context);
                }
                ImGui::TreePop();
            }
        }
    }

    CmEditorLayer::~CmEditorLayer() {
        Destroy();
    }

    void CmEditorLayer::Init(CmEditorContext *context) {
        mContext = context;
        mTitleBar.Init(context, &bShowStats, &bShowContent, &bShowDemoWindow);
    }

    void CmEditorLayer::OnImGuiRender() {
        if(!mContext){
            return;
        }

        mTitleBar.OnImGuiRender();
        DrawDockSpace();
        DrawViewportPanel();
        DrawScenePanel();
        DrawInspectorPanel();
        if(bShowContent){
            DrawContentPanel();
        }
        if(bShowStats){
            DrawStatsPanel();
        }
        if(bShowDemoWindow){
            ImGui::ShowDemoWindow(&bShowDemoWindow);
        }
    }

    void CmEditorLayer::Destroy() {
        mTitleBar.Destroy();
        mContext = nullptr;
        bDockLayoutInitialized = false;
    }

    void CmEditorLayer::DrawDockSpace() {
        ImGuiID dockSpaceId = ImGui::GetID("CmEditorDockSpace");
        ImGuiDockNodeFlags dockSpaceFlags = ImGuiDockNodeFlags_None;
        dockSpaceFlags |= ImGuiDockNodeFlags_PassthruCentralNode;

        bool shouldResetLayout = !bDockLayoutInitialized && ImGui::DockBuilderGetNode(dockSpaceId) == nullptr;
        ImGui::DockSpaceOverViewport(dockSpaceId, nullptr, dockSpaceFlags);

        if(shouldResetLayout){
            ResetDockLayout(dockSpaceId);
        }
        bDockLayoutInitialized = true;
    }

    void CmEditorLayer::DrawViewportPanel() {
        ImGui::Begin("Viewport");
        ImVec2 availableRegion = ImGui::GetContentRegionAvail();
        uint32_t width = static_cast<uint32_t>(std::max(0.f, availableRegion.x));
        uint32_t height = static_cast<uint32_t>(std::max(0.f, availableRegion.y));
        mContext->SetViewportSize(width, height);
        mContext->SetViewportFocused(ImGui::IsWindowFocused());
        mContext->SetViewportHovered(ImGui::IsWindowHovered());
        mContext->SetBlockSceneInput(ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard);

        uint64_t textureId = mContext->GetViewportTextureId();
        if(textureId != 0 && width > 0 && height > 0){
            ImGui::Image(static_cast<ImTextureID>(textureId), availableRegion);
        } else {
            ImGui::Text("Viewport");
            ImGui::Separator();
            ImGui::Text("Scene render target is not ready.");
            ImGui::Text("Current size: %u x %u", width, height);
        }
        ImGui::End();
    }

    void CmEditorLayer::DrawScenePanel() {
        ImGui::Begin("Scene Hierarchy");
        CmScene *scene = mContext->GetScene();
        if(scene){
            DrawNode(scene->GetRootNode(), mContext);
        } else {
            ImGui::Text("No scene loaded.");
        }
        ImGui::End();
    }

    void CmEditorLayer::DrawInspectorPanel() {
        ImGui::Begin("Inspector");
        CmEntity *entity = mContext->GetSelectedEntity();
        if(!CmEntity::IsValid(entity)){
            ImGui::Text("No entity selected.");
            ImGui::End();
            return;
        }

        std::string name = entity->GetName();
        char nameBuffer[256] = {};
        std::snprintf(nameBuffer, sizeof(nameBuffer), "%s", name.c_str());
        if(ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))){
            entity->SetName(nameBuffer);
        }

        if(entity->HasComponent<CmTransformComponent>()){
            auto &transform = entity->GetComponent<CmTransformComponent>();
            if(ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)){
                ImGui::DragFloat3("Position", &transform.position.x, 0.01f);
                ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f);
                ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f);
            }
        }
        ImGui::End();
    }

    void CmEditorLayer::DrawContentPanel() {
        ImGui::Begin("Content", &bShowContent);
        ImGui::Text("Content Browser");
        ImGui::Separator();
        ImGui::Text("Assets and project files will live here.");
        ImGui::End();
    }

    void CmEditorLayer::DrawStatsPanel() {
        ImGui::Begin("Stats", &bShowStats);
        CmApplication *app = mContext->GetApp();
        if(app){
            ImGui::Text("Frame: %llu", static_cast<unsigned long long>(app->GetFrameIndex()));
            ImGui::Text("Time: %.2f s", app->GetStartTimeSecond());
        }
        ImGuiIO &io = ImGui::GetIO();
        ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Mode: %s", mContext->GetMode() == CmEditorMode::Edit ? "Edit" : "Play");
        ImGui::End();
    }

    void CmEditorLayer::ResetDockLayout(uint32_t dockSpaceId) {
        ImGuiViewport *viewport = ImGui::GetMainViewport();

        ImGui::DockBuilderRemoveNode(dockSpaceId);
        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockSpaceId, viewport->WorkSize);

        ImGuiID mainNode = dockSpaceId;
        ImGuiID leftNode = 0;
        ImGuiID rightNode = 0;
        ImGuiID bottomNode = 0;

        ImGui::DockBuilderSplitNode(mainNode, ImGuiDir_Left, 0.22f, &leftNode, &mainNode);
        ImGui::DockBuilderSplitNode(mainNode, ImGuiDir_Right, 0.26f, &rightNode, &mainNode);
        ImGui::DockBuilderSplitNode(mainNode, ImGuiDir_Down, 0.28f, &bottomNode, &mainNode);

        ImGui::DockBuilderDockWindow("Scene Hierarchy", leftNode);
        ImGui::DockBuilderDockWindow("Inspector", rightNode);
        ImGui::DockBuilderDockWindow("Content", bottomNode);
        ImGui::DockBuilderDockWindow("Stats", bottomNode);
        ImGui::DockBuilderDockWindow("Viewport", mainNode);

        ImGui::DockBuilderFinish(dockSpaceId);
    }
}
