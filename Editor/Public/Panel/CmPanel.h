#pragma once

namespace chimi{
    class CmEditorContext;

    class CmPanel{
    public:
        virtual ~CmPanel() = default;

        void SetContext(CmEditorContext *context) { mContext = context; }
        CmEditorContext *GetContext() const { return mContext; }

        virtual const char *GetName() const = 0;
        virtual void OnImGuiRender() = 0;
    private:
        CmEditorContext *mContext = nullptr;
    };
}
