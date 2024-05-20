#pragma once

class UIManager
{
public:
    void HandleSphereUI();
    void HandleLightingUI() const;
    void HandlePointLightUI() const;
    void HandleDirectionalLightUI() const;
    void HandleSpotLightUI() const;
    void HandleAreaLightUI() const;
    void HandleAmbientLightUI() const;
    void HandleSkydomeUI();
    void HandleCameraUI(Camera& camera);
    void HandleRenderUI(float deltaTime);
    void HandleMaterialsUI() const;
    void HandleAllUI(float deltaTime, Camera& camera);
    void HandleScoreUI() const;

    Scene* scene;
};

