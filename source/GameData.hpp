#pragma once

#include <windows.h>

class GameData {
public:
    GameData() = default;
    virtual ~GameData() = 0;
    virtual VOID setData(PUINT8) = 0;
    virtual UINT8 *getData() = 0;
};
