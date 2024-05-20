#pragma once

struct CurrentSpecialLight
{
    uint index;
    bool bActivated = false;
};

class SpecialLights
{
public:
    SpecialLights(Scene* scene);
    void Tick(float deltaTime);
    void Reset();
    void Win();
    void CheckSpecialLight(uint hitVoxelIndex);
    void GeneratePattern();

    float baseUpdateInterval = 1.f;
    float lowestUpdateInterval = 0.5f;
    float changeInterval = 0.1f;
    float updateInterval = baseUpdateInterval;
    float baseTimeToWin = 30.f;
    float lowestTimeToWin = 10.f;
    float changeTimeToWin = 2.f;
    float timeToWin = baseTimeToWin;
    float winResetTime = 3.f;
    vector<int> pattern;
    
    Scene* scene;
    vector<uint> specialLightIndices;
    vector<CurrentSpecialLight> currentSpecialLightIndices;
    int currentPatternIndex = 0;
    int currentPattern = 0;
    bool bActivated = false;
    bool bWon = false;
    bool bReset = false;

    int score = 0;

    float timeSinceLastUpdate = 0;
    float timeRemainingToWin = timeToWin;
};
