#pragma once
#include <memory>
#include <thread>

#include "../../HGame.h"
#include "../Components/ItemComponent.h"
#include "EckyCS/ECS.h"
#include "EckyCS/Components/Base/TransformComponent.h"
#include "EckyCS/Systems/System.h"
#include "../ext/imgui/imgui.h"
#include "Renderer/Renderer.h"

using namespace EckyCS;
using namespace std;
class ItemMovementSystem : public System
{
public:

    void FixedTick(float Delta) override
    {
        if (!Cam)
            return;

        bool bIsPressed = Cam->GetKey(GLFW_KEY_SPACE) == GLFW_PRESS;
        bShouldTrigger = (bIsPressed && !bWasPressed) ? !bShouldTrigger : bShouldTrigger;
        bWasPressed = bIsPressed;
        if (!bShouldTrigger)
            return;
        
        EntityAction Action = [this](ComponentGroupIdentifier GroupID, size_t Count, View<ItemComponent, TransformComponent>& Data) -> bool {
            return this->CreateThreads(GroupID, Count, Data);
        };
        Ecs->ForEach<ItemComponent, TransformComponent>(Action);
    }

    void StartSystem(shared_ptr<ECS>& Ptr) override
    {
        Ecs = Ptr;
        Cam = Game::GetRenderer()->GetCamera();
    }

    void OnDrawGizmos(const shared_ptr<Gizmos>& Gizmos) override
    {
        if (!Cam)
            return;

        glm::vec2 Pos;
        Cam->GetMouseCoords(Pos);
        auto WorldPos = Cam->GetMouseWorldPos();
        ImGui::Begin("IMS");
        ImGui::Text("Pos: %.2f|%.2f", Pos.x, Pos.y);
        ImGui::Text("World: %.2f|%.2f|%.2f", WorldPos.x, WorldPos.y, WorldPos.z);
        ImGui::Text("Enabled: %s", (bShouldTrigger ? "true" : "false"));
        ImGui::End();
    }

private:
    shared_ptr<ECS> Ecs;
    shared_ptr<Camera> Cam;
    bool bShouldTrigger = false;
    bool bWasPressed = false;

    bool CreateThreads(ComponentGroupIdentifier GroupID, size_t Count, View<ItemComponent, TransformComponent>& Data)
    {
        size_t ThreadCount = Count / 10000;

        vector<thread> Threads;
        Threads.reserve(ThreadCount);

        glm::vec2 MousePos;
        Cam->GetMouseCoords(MousePos);
        auto CSP = Cam->Projection * Cam->View;

        for (size_t i = 0; i < ThreadCount; i++) {
            size_t Start = i * 10000;
            size_t End = (i + 1) * 10000;
            Threads.emplace_back(&ItemMovementSystem::WorkOn, ref(Data), Start, End, ref(MousePos), ref(CSP));
        }
        for (size_t i = 0; i < ThreadCount; i++)
        {
            Threads[i].join();
        }
        return true;
    }

    static void WorkOn(View<ItemComponent, TransformComponent>& Data, size_t Start, size_t End, const glm::vec2& MousePos, const glm::mat4& CSP)
    {
        auto Transforms = Get<TransformComponent>(Data).data();
        auto Items = Get<ItemComponent>(Data).data();
        for (size_t i = Start; i < End; i++)
        {
            auto ClipPos = CSP * glm::vec4(Transforms[i].PosX, Transforms[i].PosY, Transforms[i].PosZ, 1);
            auto NDCPos = glm::vec3(ClipPos) / ClipPos.w;
            glm::vec2 ScreenPos;
            ScreenPos.x = (NDCPos.x + 1.0f) * 0.5f * 1920;
            ScreenPos.y = (1.0f - NDCPos.y) * 0.5f * 1080; 

            if (glm::distance(ScreenPos, MousePos) < 150)
            {
                Transforms[i].PosX += 1;
                Items[i].Type = 5;
            }else
           {
               Items[i].Type = 1;
           }
        }
    }
};
