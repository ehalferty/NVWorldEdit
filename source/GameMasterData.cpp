#include "GameMasterData.hpp"

void GameMasterData::setData(PUINT8 fileData) {
    this->fileData = fileData;
}

PUINT8 GameMasterData::getData() {
    return this->fileData;
}
