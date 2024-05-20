#include "precomp.h"
#include "uiManager.h"

#include "game/specialLights.h"
#include "lights/lightManager.h"
#include "primitives/bvh.h"


void UIManager::HandleAllUI(float deltaTime, Camera& camera)
{
    HandleRenderUI(deltaTime);
    HandleCameraUI(camera);
    HandleSphereUI();
    HandleLightingUI();
    HandleSkydomeUI();
}

void UIManager::HandleScoreUI() const
{
    ImGui::Text("Click (Left Click) on the lights in the correct order");
    ImGui::Text("Clicking the wrong light will reset the score");
    ImGui::Text("Press P to toggle photo mode");
    ImGui::Text("Time to Win: %f", scene->specialLights->timeRemainingToWin);
    ImGui::Text("Score: %d", scene->specialLights->score);
}

Material material;

void UIManager::HandleSphereUI()
{
    if (!ImGui::CollapsingHeader("Sphere")) return;
    const char* items[] = { "Lambertian", "Diffuse", "Glossy", "Dielectric" };

    int selectedIndex = static_cast<int>(material.type);
    ImGui::ListBox("Sphere Type", &selectedIndex, items, IM_ARRAYSIZE(items));
    material.type = static_cast<Material::Type>(selectedIndex);

    switch (material.type)
    {
    case Material::Type::Dielectric:
        ImGui::SliderFloat("Dielectric Refraction Index", &material.dielectric.refractiveIndex, 0.0f, 10.0f);
        break;
    case Material::Type::Glossy:
        ImGui::SliderFloat("Glossiness", &material.glossy.fuzz, 0.0f, 1.0f);
        break;
    default:
        break;
    }
    
    if (ImGui::Button("Add Sphere"))
    {
        scene->spheres.emplace_back(new Sphere{ float3(0.0f), 0.2f, material, 0xffffff });
        delete scene->bvhSpheres;
        scene->bvhSpheres = new BVHSphere(scene->spheres);
    }
    for (int i = 0; i < scene->spheres.size(); i++)
    {
        const auto& sphere = scene->spheres[i];
        ImGui::PushID(i);
        ImGui::Text("Sphere %d", i);
        ImGui::DragFloat3("Sphere Center", &sphere->center.x, 0.1f, -10.0f, 10.0f);
        ImGui::DragFloat("Sphere Radius", &sphere->radius, 0.1f, -10.0f, 10.0f);
        auto& color = sphere->color;
        float3 colorVec = Math::GetColorNormalised(color);
        ImGui::ColorEdit3("Sphere Color", &colorVec.x);
        color = Math::GetColor(colorVec);
        if (ImGui::Button("Remove Sphere"))
        {
            scene->spheres.erase(scene->spheres.begin() + i);
            delete scene->bvhSpheres;
            scene->bvhSpheres = new BVHSphere(scene->spheres);
            i--;
        }
        ImGui::PopID();
    }
}

void UIManager::HandleLightingUI() const
{
    if (!ImGui::CollapsingHeader("Lighting")) return;
    if (ImGui::Button("Stochastic Lighting"))
    {
        scene->lightManager->bStochastic = !scene->lightManager->bStochastic;
    }
    ImGui::Text(scene->lightManager->bStochastic ? "Stochastic Lighting Enabled" : "Stochastic Lighting Disabled");
    HandlePointLightUI();
    HandleDirectionalLightUI();
    HandleSpotLightUI();
    HandleAreaLightUI();
    HandleAmbientLightUI();
    HandleMaterialsUI();
}

void UIManager::HandlePointLightUI() const
{
    if (!ImGui::CollapsingHeader("Point Lights")) return;
    auto& pointLights = scene->lightManager->pointLights;

    if (ImGui::Button("Add Point Light")) pointLights.push_back(PointLightData{ float3(0.0f), float3(1.0f), 1.0f });
    
    for (int i = 0; i < pointLights.size(); i++)
    {
        ImGui::PushID(i + 1000);
        ImGui::DragFloat("Intensity", &pointLights[i].intensity, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat3("Position", &pointLights[i].position.x, 0.1f,-10.0f, 10.0f);
        ImGui::ColorEdit3("Color", &pointLights[i].color.x);
        if (ImGui::Button("Remove Point Light"))
        {
            pointLights.erase(pointLights.begin() + i);
            i--;
        }
        ImGui::PopID();
    }
}

void UIManager::HandleDirectionalLightUI() const
{
    auto& directionalLights = scene->lightManager->directionalLights;
    if (!ImGui::CollapsingHeader("Directional Lights")) return;

    if (ImGui::Button("Add Directional Light")) directionalLights.push_back(DirectionalLightData{ float3(0.0f), float3(1.0f), 1.0f });

    for (int i = 0; i < directionalLights.size(); i++)
    {
        ImGui::PushID(i + 2000);
        ImGui::DragFloat("Intensity", &directionalLights[i].intensity,0.1f, 0.0f, 100.0f);
        ImGui::DragFloat3("Direction", &directionalLights[i].direction.x,0.1f, -1.0f, 1.0f);
        ImGui::ColorEdit3("Color", &directionalLights[i].color.x);
        if(ImGui::Button("Remove Directional Light"))
        {
            directionalLights.erase(directionalLights.begin() + i);
            i--;
        }
        ImGui::PopID();
    }
}

void UIManager::HandleSpotLightUI() const
{
    auto& spotLights = scene->lightManager->spotLights;
    if (!ImGui::CollapsingHeader("Spot Lights")) return;

    if (ImGui::Button("Add Spot Light")) spotLights.push_back(SpotLightData{
        float3(0.0f), float3(0.0f), float3(1.0f), 1.0f, 45.0f
    });
    
    for (int i = 0; i < spotLights.size(); i++)
    {
        ImGui::PushID(i + 3000);
        ImGui::DragFloat("Intensity", &spotLights[i].intensity, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat3("Position", &spotLights[i].position.x, 0.1f, -10.0f, 10.0f);
        ImGui::DragFloat3("Direction", &spotLights[i].direction.x, 0.1f, -1.0f, 1.0f);
        ImGui::DragFloat("FOV", &spotLights[i].fov, 0.1f, 0.0f, 180.0f);
        ImGui::ColorEdit3("Color", &spotLights[i].color.x);
        if (ImGui::Button("Remove Spot Light"))
        {
            spotLights.erase(spotLights.begin() + i);
            i--;
        }
        ImGui::PopID();
    }
}

void UIManager::HandleAreaLightUI() const
{
    auto& areaLights = scene->lightManager->areaLights;
    if (!ImGui::CollapsingHeader("Area Lights")) return;

    if (ImGui::Button("Add Area Light")) areaLights.push_back(AreaLightData{
        float3(0.0f), float3(1.0f), 1.0f, {1.0f}
    });
    
    for (int i = 0; i < areaLights.size(); i++)
    {
        ImGui::PushID(i + 4000);
        ImGui::DragFloat("Intensity", &areaLights[i].intensity, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat3("Position", &areaLights[i].position.x, 0.1f, -10.0f, 10.0f);
        ImGui::DragFloat2("Size", &areaLights[i].size.x, 0.1f, 0.f, 100.0f);
        ImGui::ColorEdit3("Color", &areaLights[i].color.x);
        if (ImGui::Button("Remove Area Light"))
        {
            areaLights.erase(areaLights.begin() + i);
            i--;
        }
        ImGui::PopID();
    }
}

void UIManager::HandleAmbientLightUI() const
{
    auto& ambientLight = scene->lightManager->ambientLight;
    if (!ImGui::CollapsingHeader("Ambient Light")) return;

    ImGui::DragFloat("Intensity", &ambientLight.intensity, 0.1f, 0.0f, 10.0f);
    ImGui::ColorEdit3("Color", &ambientLight.color.x);
}

void UIManager::HandleSkydomeUI()
{
}

void UIManager::HandleCameraUI(Camera& camera)
{
    if (!ImGui::CollapsingHeader("Camera")) return;
    ImGui::DragFloat3("Camera Position", &camera.camPos.x, 0.1f,-10.0f, 10.0f);
    ImGui::DragFloat3("Camera Target", &camera.camTarget.x, 0.1f,-10.0f, 10.0f);
    ImGui::DragFloat("Camera Speed", &camera.camSpeed, 0.0f, 10.0f);
    ImGui::DragFloat("Camera Focal Length", &camera.focalLength, 0.1f, 0, 10);
    ImGui::DragFloat("Camera Aperture", &camera.apertureRadius, 0.01f,0, 10);
    ImGui::Checkbox("Accumulate", &camera.bAccumulate);
    ImGui::SliderInt("Frames to Accumulate", &camera.numFramesToAccumulate, 1, 8);
}

void UIManager::HandleRenderUI(const float deltaTime)
{
    if (!ImGui::CollapsingHeader("Rendering Information")) return;
    ImGui::Text("Frame Time: %f", deltaTime);
    ImGui::Text("Frame Rate: %f", 1.0f / deltaTime);
    ImGui::Text("Resolution: %d x %d", SCRWIDTH, SCRHEIGHT);
    ImGui::Text("Million Rays/s: %f", (SCRWIDTH * SCRHEIGHT) / deltaTime / 1000000);
}

void UIManager::HandleMaterialsUI() const
{

}

