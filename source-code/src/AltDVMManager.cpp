#include "AltDVMManager.hpp"
#include <cmath>
#include <limits>

using namespace std;

AltDVMManager::AltDVMManager() {
}

void AltDVMManager::addDVM(const string& dvmId, int coorX, int coorY, const string& availability) {
    if (availability == "T") {
        DVMList.emplace_back(dvmId, coorX, coorY);
    }
}

bool AltDVMManager::selectAltDVM(int currX, int currY) {
    if (DVMList.empty()) {
        return false;
    } else {
        int minDist = numeric_limits<int>::max();
        for (auto& dvm : DVMList) {
            int x = dvm.getLocation().first;
            int y = dvm.getLocation().second;
            int dist = (currX - x) * (currX - x) + (currY - y) * (currY - y);
            if (dist < minDist) {
                minDist = dist;
                selectedDVMId = dvm.getId();
            }
        }
        return true;
    }
}

string AltDVMManager::getSelectedDVM() const {
    return selectedDVMId;
}

void AltDVMManager::reset() {
    DVMList.clear();
    selectedDVMId.clear();
}

std::pair<int,int> AltDVMManager::getAltDVMLocation() const{
    for(int iter = 0; iter<DVMList.size(); iter++){
        if(DVMList[iter].getId().compare(selectedDVMId)==0){
            return DVMList[iter].getLocation();
        }
    }
    return {-1,-1};
}

std::vector<AltDVM> AltDVMManager::getAltDVMList()
{
    return DVMList;
}