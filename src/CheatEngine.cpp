#include "CheatEngine.hpp"

CheatEngine::CheatEngine() {}
CheatEngine::~CheatEngine() {}

CheatEngine &CheatEngine::GetInstance()
{
    static CheatEngine *instance = new CheatEngine;
    return *instance;
}
void CheatEngine::MainLoop() {
    // ImGui::Begin("Test",NULL);
    // ImGui::Text("aaaaaaaaaa");
    // ImGui::End();
}

void CheatEngine::ScanForValue() {}
void CheatEngine::Rescan() {}
unsigned long CheatEngine::FindPlayerStructAddress() {}
void CheatEngine::FreezeHealth() {}