#include "precomp.h"
#include "specialLights.h"

SpecialLights::SpecialLights(Scene* scene): scene(scene)
{
    GeneratePattern();
    specialLightIndices.resize(6);
    currentSpecialLightIndices.resize(1);

    for (auto i = 0; i < 6; i++)
    {
        specialLightIndices[i] = scene->specialVoxels[i];
    }

    currentSpecialLightIndices[0].index = specialLightIndices[pattern[0]];
}

void SpecialLights::Tick(float deltaTime)
{
    if (!bWon)
    {
        timeSinceLastUpdate += deltaTime;
        timeRemainingToWin -= deltaTime;

        if (timeRemainingToWin <= 0)
        {
            timeRemainingToWin = timeToWin;
            score = 0;
            Reset();
        }
        
        if (timeSinceLastUpdate >= updateInterval)
        {
            timeSinceLastUpdate -= updateInterval;

            if (bReset)
            {
                bReset = false;
                for (const auto specialLightIndex : specialLightIndices)
                {
                    scene->grid[specialLightIndex].specialColor = 0;
                }
            }
            
            if (bActivated)
            {
                const auto lightIndex = specialLightIndices[currentPattern];
                scene->grid[lightIndex].color = 0xffffff;
            }
            else
            {
                currentPattern = pattern[currentPatternIndex];
                const auto lightIndex = specialLightIndices[currentPattern];
                scene->grid[lightIndex].color = 0x00ffff;
            }
            bActivated = !bActivated;
        }
    }
    else
    {
        timeSinceLastUpdate += deltaTime;
        if (timeSinceLastUpdate >= winResetTime)
        {
            bWon = false;
            GeneratePattern();
            Reset();
        }
    }
}

void SpecialLights::Reset()
{
    bReset = true;
    currentPatternIndex = 0;
    for (auto i = 0; i < 6; i++)
    {
        const auto lightIndex = specialLightIndices[i];
        scene->grid[lightIndex].color = 0xffffff;
        scene->grid[lightIndex].specialColor = 0xff0000;
    }
    currentSpecialLightIndices.clear();
    currentSpecialLightIndices.resize(1);
    currentSpecialLightIndices[0].index = specialLightIndices[pattern[0]];
}

void SpecialLights::Win()
{
    for (const unsigned int lightIndex : specialLightIndices)
    {
        scene->grid[lightIndex].color = 0x00ff00;
        scene->grid[lightIndex].specialColor = 0x00ff00;
    }
    bWon = true;
    score++;
    timeToWin -= changeTimeToWin;
    Math::Clamp(timeToWin, lowestTimeToWin, baseTimeToWin);
    timeRemainingToWin = timeToWin;
    updateInterval -= changeInterval;
    Math::Clamp(updateInterval, lowestUpdateInterval, baseUpdateInterval);
    cout << "You win!" << '\n';
}
    
void SpecialLights::CheckSpecialLight(const uint hitVoxelIndex)
{
    for (auto i = 0; i < static_cast<int>(currentSpecialLightIndices.size()); i++)
    {
        if (hitVoxelIndex == currentSpecialLightIndices[i].index)
        {
            currentSpecialLightIndices[i].bActivated = true;
            const auto lightIndex = currentSpecialLightIndices[i].index;
            scene->grid[lightIndex].specialColor = 0x00ffff;
            currentSpecialLightIndices.resize((i + 2));
            if (currentSpecialLightIndices.size() == 7)
            {
                Win();
                return;
            }
            for (auto j = 0; j < static_cast<int>(currentSpecialLightIndices.size()); j++)
            {
                currentSpecialLightIndices[j].index = specialLightIndices[pattern[j]];
            }
            timeSinceLastUpdate = 0;
            currentPatternIndex++;
            return;
        }
        if (currentSpecialLightIndices[i].bActivated == false)
        {
            Reset();
            timeRemainingToWin = baseTimeToWin;
            timeToWin = baseTimeToWin;
            updateInterval = baseUpdateInterval;
            score = 0;
            break;  
        }
    }
}

void SpecialLights::GeneratePattern()
{
    pattern.clear();
    while (pattern.size() < 6)
    {
        const auto random = Math::RandomIntRange(0, 5);
        if (find(pattern.begin(), pattern.end(), random) == pattern.end())
        {
            pattern.push_back(random);
        }
    }
}
