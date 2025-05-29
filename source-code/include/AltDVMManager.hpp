#pragma once
#include <vector>
#include <string>
#include "AltDVM.hpp"
#include "ItemManager.hpp"

class AltDVMManager {
private:
    std::vector<AltDVM> DVMList;
    std::string selectedDVMId;

public:
    AltDVMManager();
    void addDVM(const std::string& dvmId, int coorX, int coorY, const std::string& availability);
    void selectAltDVM(int currX, int currY); // 현재 자판기 위치를 받아 선택
    std::string getSelectedDVM() const;
    std::pair<int,int> getAltDVMLocation() const;
    void reset();

    // google test용 메서드
    std::vector<AltDVM> getAltDVMList();
};
