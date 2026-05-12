#include "CmEditorTitleBar.h"

#include "CmApplication.h"
#include "CmEditorContext.h"
#include "CmWindow.h"

#include "imgui.h"

namespace chimi{
    namespace{
        constexpr float TITLE_BAR_HEIGHT = 28.f;
        constexpr float RESIZE_BORDER_SIZE = 6.f;
        constexpr int MIN_WINDOW_WIDTH = 640;
        constexpr int MIN_WINDOW_HEIGHT = 360;

        enum ResizeEdge : uint32_t{
            RESIZE_EDGE_NONE = 0,
            RESIZE_EDGE_LEFT = 1 << 0,
            RESIZE_EDGE_RIGHT = 1 << 1,
            RESIZE_EDGE_TOP = 1 << 2,
            RESIZE_EDGE_BOTTOM = 1 << 3
        };

        bool HasResizeEdge(uint32_t edges, ResizeEdge edge){
            return (edges & edge) != 0;
        }

        CmWindow *GetWindow(CmEditorContext *context){
            return context && context->GetApp() ? context->GetApp()->GetWindow() : nullptr;
        }
    }

    void CmEditorTitleBar::Init(CmEditorContext *context, bool *showStats, bool *showContent, bool *showDemoWindow) {
        mContext = context;
        bShowStats = showStats;
        bShowContent = showContent;
        bShowDemoWindow = showDemoWindow;
    }

    void CmEditorTitleBar::OnImGuiRender() {
        if(!mContext){
            return;
        }

        HandleWindowResize();

        if(ImGui::BeginMainMenuBar()){
            ImGuiViewport *viewport = ImGui::GetMainViewport();
            float buttonWidth = TITLE_BAR_HEIGHT + 6.f;
            float controlWidth = buttonWidth * 3.f;

            ImGui::TextUnformatted("Chimi Engine");
            ImGui::SameLine();
            DrawMenu();

            float dragStartX = ImGui::GetCursorPosX();
            float dragWidth = viewport->Size.x - dragStartX - controlWidth - 12.f;
            if(dragWidth > 0.f){
                ImGui::SameLine();
                ImGui::InvisibleButton("##CmEditorTitleBarDrag", ImVec2(dragWidth, ImGui::GetFrameHeight()));
                HandleWindowDrag();
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX(viewport->Size.x - controlWidth);
            DrawWindowButtons();

            ImGui::EndMainMenuBar();
        }
    }

    void CmEditorTitleBar::Destroy() {
        mContext = nullptr;
        bShowStats = nullptr;
        bShowContent = nullptr;
        bShowDemoWindow = nullptr;
        bDraggingWindow = false;
        bResizingWindow = false;
        mResizeEdges = RESIZE_EDGE_NONE;
    }

    void CmEditorTitleBar::DrawMenu() {
        CmApplication *app = mContext->GetApp();
        if(ImGui::BeginMenu("File")){
            ImGui::MenuItem("New Scene");
            ImGui::MenuItem("Open Scene");
            ImGui::MenuItem("Save Scene");
            ImGui::Separator();
            if(ImGui::MenuItem("Exit") && app){
                app->Close();
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Window")){
            if(bShowContent){
                ImGui::MenuItem("Content", nullptr, bShowContent);
            }
            if(bShowStats){
                ImGui::MenuItem("Stats", nullptr, bShowStats);
            }
            if(bShowDemoWindow){
                ImGui::MenuItem("ImGui Demo", nullptr, bShowDemoWindow);
            }
            ImGui::EndMenu();
        }
    }

    void CmEditorTitleBar::DrawWindowButtons() {
        CmWindow *window = GetWindow(mContext);
        CmApplication *app = mContext->GetApp();
        float buttonWidth = TITLE_BAR_HEIGHT + 6.f;
        ImVec2 buttonSize{ buttonWidth, ImGui::GetFrameHeight() };

        if(ImGui::Button("_", buttonSize) && window){
            window->Minimize();
        }
        ImGui::SameLine(0.f, 0.f);

        const char *maximizeLabel = window && window->IsMaximized() ? "[]" : "[ ]";
        if(ImGui::Button(maximizeLabel, buttonSize)){
            ToggleMaximized();
        }
        ImGui::SameLine(0.f, 0.f);

        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.82f, 0.18f, 0.18f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.08f, 0.08f, 1.f });
        if(ImGui::Button("X", buttonSize) && app){
            app->Close();
        }
        ImGui::PopStyleColor(2);
    }

    void CmEditorTitleBar::HandleWindowDrag() {
        CmWindow *window = GetWindow(mContext);
        if(!window){
            return;
        }

        ImGuiIO &io = ImGui::GetIO();
        if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)){
            ToggleMaximized();
            return;
        }

        if(ImGui::IsItemActivated() && !window->IsMaximized()){
            bDraggingWindow = true;
            mDragStartMousePos = { io.MousePos.x, io.MousePos.y };
            window->GetPosition(mDragStartWindowPos);
        }

        if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)){
            bDraggingWindow = false;
        }

        if(bDraggingWindow){
            glm::vec2 delta = { io.MousePos.x - mDragStartMousePos.x, io.MousePos.y - mDragStartMousePos.y };
            window->SetPosition({
                static_cast<int>(mDragStartWindowPos.x + delta.x),
                static_cast<int>(mDragStartWindowPos.y + delta.y)
            });
        }
    }

    void CmEditorTitleBar::HandleWindowResize() {
        CmWindow *window = GetWindow(mContext);
        if(!window || window->IsMaximized()){
            bResizingWindow = false;
            return;
        }

        glm::ivec2 windowPos;
        glm::ivec2 windowSize;
        window->GetPosition(windowPos);
        window->GetSize(windowSize);

        ImGuiIO &io = ImGui::GetIO();
        glm::vec2 mousePos = { io.MousePos.x, io.MousePos.y };
        uint32_t edges = RESIZE_EDGE_NONE;

        bool insideX = mousePos.x >= windowPos.x && mousePos.x <= windowPos.x + windowSize.x;
        bool insideY = mousePos.y >= windowPos.y && mousePos.y <= windowPos.y + windowSize.y;
        if(insideY && mousePos.x >= windowPos.x && mousePos.x <= windowPos.x + RESIZE_BORDER_SIZE){
            edges |= RESIZE_EDGE_LEFT;
        }
        if(insideY && mousePos.x >= windowPos.x + windowSize.x - RESIZE_BORDER_SIZE && mousePos.x <= windowPos.x + windowSize.x){
            edges |= RESIZE_EDGE_RIGHT;
        }
        if(insideX && mousePos.y >= windowPos.y && mousePos.y <= windowPos.y + RESIZE_BORDER_SIZE){
            edges |= RESIZE_EDGE_TOP;
        }
        if(insideX && mousePos.y >= windowPos.y + windowSize.y - RESIZE_BORDER_SIZE && mousePos.y <= windowPos.y + windowSize.y){
            edges |= RESIZE_EDGE_BOTTOM;
        }

        if(edges != RESIZE_EDGE_NONE || bResizingWindow){
            if((HasResizeEdge(edges, RESIZE_EDGE_LEFT) && HasResizeEdge(edges, RESIZE_EDGE_TOP)) ||
               (HasResizeEdge(edges, RESIZE_EDGE_RIGHT) && HasResizeEdge(edges, RESIZE_EDGE_BOTTOM))){
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
            } else if((HasResizeEdge(edges, RESIZE_EDGE_RIGHT) && HasResizeEdge(edges, RESIZE_EDGE_TOP)) ||
                      (HasResizeEdge(edges, RESIZE_EDGE_LEFT) && HasResizeEdge(edges, RESIZE_EDGE_BOTTOM))){
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
            } else if(HasResizeEdge(edges, RESIZE_EDGE_LEFT) || HasResizeEdge(edges, RESIZE_EDGE_RIGHT)){
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            } else {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            }
        }

        if(!bResizingWindow && edges != RESIZE_EDGE_NONE && ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
            bResizingWindow = true;
            mResizeEdges = edges;
            mResizeStartMousePos = mousePos;
            mResizeStartWindowPos = windowPos;
            mResizeStartWindowSize = windowSize;
        }

        if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)){
            bResizingWindow = false;
            mResizeEdges = RESIZE_EDGE_NONE;
            return;
        }

        if(!bResizingWindow){
            return;
        }

        glm::vec2 delta = {
            mousePos.x - mResizeStartMousePos.x,
            mousePos.y - mResizeStartMousePos.y
        };
        glm::ivec2 newPos = mResizeStartWindowPos;
        glm::ivec2 newSize = mResizeStartWindowSize;

        if(HasResizeEdge(mResizeEdges, RESIZE_EDGE_LEFT)){
            newPos.x = static_cast<int>(mResizeStartWindowPos.x + delta.x);
            newSize.x = static_cast<int>(mResizeStartWindowSize.x - delta.x);
            if(newSize.x < MIN_WINDOW_WIDTH){
                newSize.x = MIN_WINDOW_WIDTH;
                newPos.x = mResizeStartWindowPos.x + mResizeStartWindowSize.x - MIN_WINDOW_WIDTH;
            }
        }
        if(HasResizeEdge(mResizeEdges, RESIZE_EDGE_RIGHT)){
            newSize.x = static_cast<int>(mResizeStartWindowSize.x + delta.x);
            newSize.x = std::max(newSize.x, MIN_WINDOW_WIDTH);
        }
        if(HasResizeEdge(mResizeEdges, RESIZE_EDGE_TOP)){
            newPos.y = static_cast<int>(mResizeStartWindowPos.y + delta.y);
            newSize.y = static_cast<int>(mResizeStartWindowSize.y - delta.y);
            if(newSize.y < MIN_WINDOW_HEIGHT){
                newSize.y = MIN_WINDOW_HEIGHT;
                newPos.y = mResizeStartWindowPos.y + mResizeStartWindowSize.y - MIN_WINDOW_HEIGHT;
            }
        }
        if(HasResizeEdge(mResizeEdges, RESIZE_EDGE_BOTTOM)){
            newSize.y = static_cast<int>(mResizeStartWindowSize.y + delta.y);
            newSize.y = std::max(newSize.y, MIN_WINDOW_HEIGHT);
        }

        window->SetPosition(newPos);
        window->SetSize(newSize);
    }

    void CmEditorTitleBar::ToggleMaximized() {
        CmWindow *window = GetWindow(mContext);
        if(!window){
            return;
        }
        if(window->IsMaximized()){
            window->Restore();
        } else {
            window->Maximize();
        }
    }
}
