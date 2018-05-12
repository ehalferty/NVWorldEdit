//
// Created by phaz on 5/11/2018.
//

#include "GameMasterData.hxx"

void GameMasterData::setData(PUINT8 fileData) {
    this->fileData = fileData;
}

PUINT8 GameMasterData::getData() {
    return this->fileData;
}
