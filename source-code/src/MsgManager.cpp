#include "MsgManager.hpp"
#include <json.hpp>
#include <iostream>

using json = nlohmann::json;

MsgManager::MsgManager(ItemManager* im, AuthCodeManager* am, AltDVMManager* adm, P2PClient* pc, const std::string& id, int x, int y)
    : itemManager(im), authCodeManager(am), altDvmManager(adm), p2pClient(pc), myId(id), coorX(x), coorY(y) {}

// 재고 확인 요청 메시지 생성
std::string MsgManager::createRequestItemStockAndLocation(int itemCode, int itemNum) {
    json j;
    j["msg_type"] = "req_stock";
    j["src_id"] = myId;
    j["dst_id"] = 0;
    j["msg_content"]["item_code"] = itemCode;
    j["msg_content"]["item_num"] = itemNum;
    return j.dump();
}

// 재고 확인 응답 메시지 생성
std::string MsgManager::createResponseItemStockAndLocation(const std::string& dstId, int itemCode, int itemNum, int coorX, int coorY) {
    json j;
    j["msg_type"] = "resp_stock";
    j["src_id"] = myId;
    j["dst_id"] = dstId;
    j["msg_content"]["item_code"] = itemCode;
    j["msg_content"]["item_num"] = itemNum;
    j["msg_content"]["coor_x"] = coorX;
    j["msg_content"]["coor_y"] = coorY;
    return j.dump();
}

// 선결제 요청 메시지 생성
std::string MsgManager::createRequestPrepayment(const std::string& dstId, int itemCode, int itemNum, const std::string& certCode) {
    json j;
    j["msg_type"] = "req_prepay";
    j["src_id"] = myId;
    j["dst_id"] = dstId;
    j["msg_content"]["item_code"] = itemCode;
    j["msg_content"]["item_num"] = itemNum;
    j["msg_content"]["cert_code"] = certCode;
    return j.dump();
}

// 선결제 가능 여부 응답 메시지 생성
std::string MsgManager::createResponsePrepayment(const std::string& dstId, int itemCode, int itemNum, const std::string& availability) {
    json j;
    j["msg_type"] = "resp_prepay";
    j["src_id"] = myId;
    j["dst_id"] = dstId;
    j["msg_content"]["item_code"] = itemCode;
    j["msg_content"]["item_num"] = itemNum;
    j["msg_content"]["availability"] = availability; // "T" 또는 "F"
    return j.dump();
}

// 메시지 전송
// void MsgManager::sendTo(const std::string& dstId, const std::string& msg) {
//     int basePort = 5000;
//     // 브로드캐스팅
//     if(dstId == "0"){
        
//     }
//     int dvmNum = std::stoi(dstId.substr(1)); // DVM id에서 숫자 추출("T2" → 2)
//     int targetPort = basePort + dvmNum;

//     std::string resp;
//     p2pClient->sendMessageToPeer("127.0.0.1", targetPort, msg, resp);
// }

void MsgManager::sendTo(const std::string& dstId, const std::string& msg) {
    int basePort = 5001;
    std::string resp;
    int targetPort = basePort;
    int dvmNum;

    if (dstId == "0") {
        // 브로드캐스팅 처리
        std::cout << "[MsgManager] 브로드캐스트로 전송: " << msg << std::endl;
        targetPort++;
        p2pClient->sendMessageToPeer("127.0.0.1", targetPort, msg, resp);
        return;
    }

    if (dstId.size() < 2 || dstId[0] != 'T') {
        std::cerr << "[MsgManager] 잘못된 dstId: " << dstId << std::endl;
        return;
    }

    try {
        dvmNum = std::stoi(dstId.substr(1));
    } catch (const std::exception& e) {
        std::cerr << "[MsgManager] dstId 파싱 실패: " << e.what() << std::endl;
        return;
    }
    targetPort += dvmNum;
    p2pClient->sendMessageToPeer("127.0.0.1", targetPort, msg, resp);
}

// 메시지 수신 및 파싱
std::string MsgManager::receive(const std::string& rawMsg) {
    try {
        json j = json::parse(rawMsg);
        std::string type = j["msg_type"].get<std::string>();

        // 재고 확인 요청
        if (type == "req_stock") {
            int itemCode = j["msg_content"]["item_code"].get<int>();
            std::string dstId = j["src_id"].get<std::string>();
            int itemNum = itemManager->checkStock(itemCode);
            
            return createResponseItemStockAndLocation(dstId, itemCode, itemNum, coorX, coorY);
        }
        // 재고 확인 응답 끝
        else if (type == "resp_stock") {
            int itemCode = j["msg_content"]["item_code"].get<int>();
            int itemNum = j["msg_content"]["item_num"].get<int>();
            int coorX = j["msg_content"]["coor_x"].get<int>();
            int coorY = j["msg_content"]["coor_y"].get<int>();
            std::string srcId = j["src_id"].get<std::string>();
            if(itemManager->getSelectedItemNum() < itemNum)
                altDvmManager->addDVM(srcId,coorX,coorY,"T");
            else
                altDvmManager->addDVM(srcId,coorX,coorY,"F");

            return "STOCK_INFO_UPDATED";
        }
        // 선결제 요청 끝
        else if (type == "req_prepay") {
            std::string dstId = j["src_id"].get<std::string>();
            int itemCode = j["msg_content"]["item_code"].get<int>();
            int itemNum = j["msg_content"]["item_num"].get<int>();
            std::string certCode = j["msg_content"]["cert_code"].get<std::string>();
            std::string availability;

            if(itemManager->isEnough(itemCode)){
                itemManager->minusStock(itemCode, itemNum);
                authCodeManager->saveAuthCode(certCode, itemCode, itemNum);
                availability = "T";
            }
            else{
                availability = "F";
            }

            return createResponsePrepayment(dstId, itemCode, itemNum, availability);
        }
        // 선결제 응답
        else if (type == "resp_prepay") {
            std::string availability = j["msg_content"]["availability"].get<std::string>();

            if(availability == "T") { // 해당 대안 자판기에 인증코드 저장이 완료된 경우
                // 선결제 성공
                return "PREPAYMENT_SUCCESS";
            } else {
                return "PREPAYMENT_FAILED";
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Error parsing message: " << e.what() << std::endl;
        return "{\"msg_type\":\"ERROR\"}";
    }

    return "{\"msg_type\":\"UNKNOWN\"}";
}