#pragma once

#include "GameData.hpp"

class GameMasterData : public GameData {
public:
    ~GameMasterData() override;
    VOID setData(PUINT8) override;
    PUINT8 getData() override;
private:
    PUINT8 fileData;
};
