#include "ui/UI.h"

UIManager::UIManager() {}
UIManager::~UIManager() {}

void UIManager::Init() {}
void UIManager::Terminate() {}

void UIManager::Poll() {}

bool UIManager::IsBlockingTransition() {}
bool UIManager::IsTimelineResetAllowed() const {}
void UIManager::SendTransitionComplete(UIScreen*, UIScreen*) {}

void UIManager::Draw() {}

void UIManager::CancelTransition() {}

void UIManager::GotoScreenImpl(UIScreen*, bool, bool) {}
void UIManager::GotoScreen(const char*, bool, bool) {}
void UIManager::GotoScreen(UIScreen*, bool, bool) {}
void UIManager::PushScreen(UIScreen*) {}
void UIManager::PopScreen(UIScreen*) {}
void UIManager::ResetScreen(UIScreen*) {}

bool UIManager::InComponentSelect() {}

UIPanel* UIManager::FocusPanel() {}
UIComponent* UIManager::FocusComponent() {}

void UIManager::Resource(const UIComponent*) {}
void UIManager::InitResources(Symbol) {}
void UIManager::FindResource(const DataArray*) {}
void UIManager::OnIsResource(DataArray*) {}

void UIManager::UseJoypad(bool, bool) {}
void UIManager::OverloadHorizontalNav(JoypadAction, JoypadButton, Symbol) const {}

void UIManager::RequireFixedText() const {}
void UIManager::SetRequireFixedText(bool) {}

void UIManager::BottomScreen() {}
void UIManager::PushDepth() const {}
void UIManager::ScreenAtDepth(int) {}

void UIManager::ToggleLoadTimes() {}
void UIManager::BlockHandlerDuringTransition(Symbol, DataArray*) {}
void UIManager::EnableInputPerformanceMode(bool) {}
void UIManager::PrintLoadedDirs(const char*) {}
void UIManager::ShowNetError() {}

DataNode UIManager::Handle(DataArray*, bool) {}

void UIManager::WentBack() const {}
void UIManager::OnGotoScreen(const DataArray*) {}
void UIManager::OnGoBackScreen(const DataArray*) {}
void UIManager::ForeachScreen(const DataArray*) {}
