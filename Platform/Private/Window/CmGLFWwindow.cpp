#include "Window/CmGLFWwindow.h"
#include "Event/CmEventDispatcher.h"
#include "CmLog.h"
#include "GLFW/glfw3native.h"

namespace chimi
{
    CmGLFWwindow::CmGLFWwindow(uint32_t width, uint32_t height, const char *title) {
        if(!glfwInit()){
            LOG_E("Failed to init glfw.");
            return;
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        mGLFWwindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if(!mGLFWwindow){
            LOG_E("Failed to create glfw window.");
            return;
        }

        GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
        if(primaryMonitor){
            int xPos, yPos, workWidth, workHeight;
            glfwGetMonitorWorkarea(primaryMonitor, &xPos, &yPos, &workWidth, &workHeight);
            glfwSetWindowPos(mGLFWwindow, workWidth / 2 - width / 2, workHeight / 2 - height / 2);
        }

        // For OpenGL/OpenGLES https://www.glfw.org/docs/latest/group__context.html#ga6d4e0cdf151b5e579bd67f13202994ed
        // This function sets the swap interval for the current OpenGL or OpenGL ES context,
        // i.e. the number of screen updates to wait from the time glfwSwapBuffers was called before swapping the buffers and returning.
        //glfwSwapInterval(0);

        glfwMakeContextCurrent(mGLFWwindow);

        // SetupWindowCallbacks();

        // show window
        glfwShowWindow(mGLFWwindow);
    }

    CmGLFWwindow::~CmGLFWwindow() {
        glfwDestroyWindow(mGLFWwindow);
        glfwTerminate();
        LOG_I("The application running end.");
    }

    bool CmGLFWwindow::ShouldClose()
    {
        return glfwWindowShouldClose(mGLFWwindow);
    }

    void CmGLFWwindow::PollEvents()
    {
        glfwPollEvents();
    }

    void CmGLFWwindow::SwapBuffer()
    {
        glfwSwapBuffers(mGLFWwindow);
    }

    void CmGLFWwindow::GetMousePos(glm::vec2 &mousePos) const {
        double x, y;
        glfwGetCursorPos(mGLFWwindow, &x, &y);
        mousePos = { x, y };
    }

    bool CmGLFWwindow::IsMouseDown(MouseButton mouseButton) const {
        return glfwGetMouseButton(mGLFWwindow, mouseButton) == GLFW_PRESS;
    }

    bool CmGLFWwindow::IsMouseUp(MouseButton mouseButton) const {
        return glfwGetMouseButton(mGLFWwindow, mouseButton) == GLFW_RELEASE;
    }

    bool CmGLFWwindow::IsKeyDown(Key key) const {
        return glfwGetKey(mGLFWwindow, key) == GLFW_PRESS;
    }

    bool CmGLFWwindow::IsKeyUp(Key key) const {
        return glfwGetKey(mGLFWwindow, key) == GLFW_RELEASE;
    }

    void CmGLFWwindow::SetupWindowCallbacks() {
        glfwSetWindowUserPointer(mGLFWwindow, this);
        glfwSetFramebufferSizeCallback(mGLFWwindow, [](GLFWwindow* window, int width, int height){
            auto *cmWindow = static_cast<CmGLFWwindow*>(glfwGetWindowUserPointer(window));
            if(cmWindow){
                CmFrameBufferResizeEvent fbResizeEvent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
                CmEventDispatcher::GetInstance()->Dispatch(fbResizeEvent);
            }
        });

        glfwSetWindowFocusCallback(mGLFWwindow, [](GLFWwindow* window, int focused){
            auto *cmWindow = static_cast<CmGLFWwindow *>(glfwGetWindowUserPointer(window));
            if(cmWindow){
                if(focused){
                    CmWindowFocusEvent windowFocusEvent{};
                    CmEventDispatcher::GetInstance()->Dispatch(windowFocusEvent);
                } else {
                    CmWindowLostFocusEvent windowLostFocusEvent{};
                    CmEventDispatcher::GetInstance()->Dispatch(windowLostFocusEvent);
                }
            }
        });

        glfwSetWindowPosCallback(mGLFWwindow, [](GLFWwindow* window, int xpos, int ypos){
            auto *cmWindow = static_cast<CmGLFWwindow *>(glfwGetWindowUserPointer(window));
            if(cmWindow){
                CmWindowMovedEvent windowMovedEvent{ static_cast<uint32_t>(xpos), static_cast<uint32_t>(ypos) };
                CmEventDispatcher::GetInstance()->Dispatch(windowMovedEvent);
            }
        });

        glfwSetWindowCloseCallback(mGLFWwindow, [](GLFWwindow* window){
            auto *cmWindow = static_cast<CmGLFWwindow *>(glfwGetWindowUserPointer(window));
            if(cmWindow){
                CmWindowCloseEvent windowCloseEvent{};
                CmEventDispatcher::GetInstance()->Dispatch(windowCloseEvent);
            }
        });

        glfwSetKeyCallback(mGLFWwindow, [](GLFWwindow* window, int key, int scancode, int action, int mods){
            auto *cmWindow = static_cast<CmGLFWwindow *>(glfwGetWindowUserPointer(window));
            if(cmWindow){
                if(action == GLFW_RELEASE){
                    CmKeyReleaseEvent keyReleaseEvent{static_cast<Key>(key) };
                    CmEventDispatcher::GetInstance()->Dispatch(keyReleaseEvent);
                } else {
                    CmKeyPressEvent keyPressEvent{static_cast<Key>(key), static_cast<KeyMod>(mods), action == GLFW_REPEAT };
                    CmEventDispatcher::GetInstance()->Dispatch(keyPressEvent);
                }
            }
        });

        glfwSetMouseButtonCallback(mGLFWwindow, [](GLFWwindow* window, int button, int action, int mods){
            auto *cmWindow = static_cast<CmGLFWwindow *>(glfwGetWindowUserPointer(window));
            if(cmWindow){
                if(action == GLFW_PRESS){
                    CmMouseButtonPressEvent mouseButtonPressEvent{static_cast<MouseButton>(button), static_cast<KeyMod>(mods), false };
                    CmEventDispatcher::GetInstance()->Dispatch(mouseButtonPressEvent);
                }
                if(action == GLFW_RELEASE){
                    CmMouseButtonReleaseEvent mouseButtonReleaseEvent{static_cast<MouseButton>(button) };
                    CmEventDispatcher::GetInstance()->Dispatch(mouseButtonReleaseEvent);
                }
            }
        });

        glfwSetCursorPosCallback(mGLFWwindow, [](GLFWwindow* window, double xpos, double ypos){
            auto *cmWindow = static_cast<CmGLFWwindow *>(glfwGetWindowUserPointer(window));
            if(cmWindow){
                CmMouseMovedEvent mouseMovedEvent{ static_cast<float>(xpos), static_cast<float>(ypos) };
                CmEventDispatcher::GetInstance()->Dispatch(mouseMovedEvent);
            }
        });

        glfwSetScrollCallback(mGLFWwindow, [](GLFWwindow* window, double xoffset, double yoffset){
            auto *cmWindow = static_cast<CmGLFWwindow *>(glfwGetWindowUserPointer(window));
            if(cmWindow){
                CmMouseScrollEvent mouseScrollEvent{ static_cast<float>(xoffset), static_cast<float>(yoffset) };
                CmEventDispatcher::GetInstance()->Dispatch(mouseScrollEvent);
            }
        });
    }
}
