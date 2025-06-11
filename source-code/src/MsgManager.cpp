#include "MsgManager.hpp"
#include <json.hpp>
#include <iostream>
#include <winsock2.h>

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

void MsgManager::sendTo(const std::string& dstId, const std::string& msg)
{
    constexpr int BASE_PORT = 5000;
    std::string resp;

    /* 람다: 한 대상 포트로 보내고, 응답을 즉시 파싱 */
    auto trySend = [&](int port) {
        if (p2pClient->sendMessageToPeer("127.0.0.1", port, msg, resp)) {
            if (!resp.empty())
                receive(resp); // 응답 처리
        }
        else {
            std::cerr << "[P2PClient] connection failed to 127.0.0.1:"
                      << port << " error code: "<< WSAGetLastError() << '\n';
        }
    };

    /* ── 1) 브로드캐스트 ── */
    if (dstId == "0") {
        std::cout << "[MsgManager] 브로드캐스트 전송\n";

        for (int num = 1; num <= 7; ++num) { // 자판기 T1~T7 로 가정
            if ("T" + std::to_string(num) == myId) continue; // 자기 자신 제외
            trySend(BASE_PORT + num);
        }
        return;
    }

    /* ── 2) 단일 전송 ── */
    if (dstId.size() >= 2 && dstId[0] == 'T') {
        int num = std::stoi(dstId.substr(1)); // "T2" → 2
        trySend(BASE_PORT + num);
    } else {
        std::cerr << "[MsgManager] 잘못된 dstId: " << dstId << '\n';
    }
}

// 메시지 수신 및 파싱
std::string MsgManager::receive(const std::string& rawMsg)
{
    std::cout << "\n[MsgManager] 수신 메시지: " << rawMsg << '\n';

    try {
        json j   = json::parse(rawMsg);
        std::string type = j["msg_type"].get<std::string>();

        /* 1) 재고 요청 수신 ------------------------------------ */
        if (type == "req_stock") {
            int itemCode  = j["msg_content"]["item_code"].get<int>();
            int itemCount = itemManager->checkStock(itemCode);
            std::string dst = j["src_id"].get<std::string>();

            return createResponseItemStockAndLocation(dst,itemCode,itemCount,coorX,coorY);
        }

        /* 2) 재고 응답 수신 ------------------------------------ */
        if (type == "resp_stock") {
            int itemCode  = j["msg_content"]["item_code"].get<int>();
            int itemNum   = j["msg_content"]["item_num"].get<int>();
            int x         = j["msg_content"]["coor_x"].get<int>();
            int y         = j["msg_content"]["coor_y"].get<int>();
            std::string src = j["src_id"].get<std::string>();

            /* 재고 충분 여부 판단 후 목록에 추가 */
            std::string avail = (itemManager->getSelectedItemNum() <= itemNum) ? "T" : "F";
            altDvmManager->addDVM(src,x,y,avail);

            return "STOCK_INFO_UPDATED";
        }

        /* 3) 선결제 요청 수신 ---------------------------------- */
        if (type == "req_prepay") {
            std::string dstId   = j["src_id"].get<std::string>();
            int itemCode        = j["msg_content"]["item_code"].get<int>();
            int itemNum         = j["msg_content"]["item_num"].get<int>();
            std::string cert    = j["msg_content"]["cert_code"].get<std::string>();
            std::string avail   = itemManager->isEnough(itemCode) ? "T" : "F";

            if (avail=="T") {
                itemManager->minusStock(itemCode,itemNum);
                authCodeManager->saveAuthCode(cert,itemCode,itemNum);
            }
            return createResponsePrepayment(dstId,itemCode,itemNum,avail);
        }

        /* 4) 선결제 응답 수신 ---------------------------------- */
        if (type == "resp_prepay") {
            return j["msg_content"]["availability"].get<std::string>() == "T"
                   ? "PREPAYMENT_SUCCESS" : "PREPAYMENT_FAILED";
        }
    }
    catch (std::exception& e) {
        std::cerr << "[MsgManager] JSON parse error: " << e.what() << '\n';
    }

    return "{\"msg_type\":\"UNKNOWN\"}";
}
