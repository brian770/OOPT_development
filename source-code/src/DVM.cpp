#include "DVM.hpp"
#include <iostream>
#include <algorithm>

using namespace std;

/** ---- 실행흐름 ----
 * -- 일반적인 음료수 구매 흐름 --
 * 메뉴 출력 -> 메뉴 선택 -> 재고 확인
 * -> (재고가 충분한 경우) -> 결제 요청
 *                    -> (결제 성공한 경우) -> 재고 감소 -> 구매 결과 출력 -> 종료
 *                    -> (결제 실패한 경우) -> 실패 이유 출력 -> 종료
 * -> (재고가 부족한 경우) -> 대안 자판기 탐색 -> 선결제 여부 확인
 *                    -> (선결제 O) -> 인증코드 생성 -> 대안자판기에게 전달
 *                    -> (선결제 X) -> 대안 자판기 위치 출력
 * 
 * -- 사용자가 선결제 코드를 입력 했을때 --
 * 인증코드 입력 -> 인증코드 확인 -> 인증코드 객체에 담겨있는 정보로 음료수 제공
 */
DVM::DVM(const string& id, int x, int y, int port)
    : DVMId(id),
    coorX(x),
    coorY(y),
    port(port),
    bank(),
    itemManager(),
    paymentManager(&bank),
    authCodeManager(),
    altDVMManager(),
    p2pClient(),

    msgManager(&itemManager, &authCodeManager, &altDVMManager, &p2pClient, id, x, y),
    p2pServer(
        port,
        [this](const std::string& msg) -> std::string {
            return msgManager.receive(msg);
        }
    )
{
    p2pServer.start();
    run();
}

void DVM::run() {
    while(true){
        std::string answer = askBuyOrCodeInput();

        if (answer == "1") {
            handleBuyFlow();
        } else if(answer == "2") {
            handleAuthCodeFlow();
        } else if(answer == "3") {
            break;
        }
    }
}

// 음료수 구매 로직
void DVM::handleBuyFlow() {
    itemManager.showItemList();
    itemManager.saveSelectedItem(requestSelect());

    // 음료수가 충분한 경우
    if (itemManager.isEnough()) {
        int payResult = paymentManager.requestPayment(itemManager.getPaymentAmount());
        showPaymentResult(payResult);

        // 결제 성공
        if (payResult == 1) {
            itemManager.minusStock();
            itemManager.showBuyResult();
        }
    // 재고가 부족한 경우
    } else {
        string tempMsg = msgManager.createRequestItemStockAndLocation(itemManager.getSelectedItemId(), itemManager.getSelectedItemNum());
        msgManager.sendTo("0",tempMsg);
        if(!altDVMManager.selectAltDVM(coorX, coorY)){
            std::cout << "대안 자판기가 없습니다." << std::endl;
            return;
        }

        // 선결제 원하는 경우
        if (askUserPrepayment() == "y") {
            string authCode = authCodeManager.generateCode();
            string dvmID = altDVMManager.getSelectedDVM();

            string tempMsg = msgManager.createRequestPrepayment(dvmID, itemManager.getSelectedItemId(), itemManager.getSelectedItemNum(), authCode);
            msgManager.sendTo(dvmID, tempMsg);

            if(msgManager.receive(tempMsg) == "PREPAYMENT_FAILED") {
                cout << "대안 자판기에서 재고가 부족합니다." << endl;
                return;
            }

            int payResult = paymentManager.requestPayment(itemManager.getPaymentAmount());
            showPaymentResult(payResult);

            // 결제 성공
            if (payResult == 1) {
                showPrepaymentResult(authCode, altDVMManager.getAltDVMLocation());
            }
        // 선결제 원하지 않는 경우
        } else {
            showPrepaymentResult("noprepayment", altDVMManager.getAltDVMLocation());
        }
    }
    altDVMManager.reset();
}

// 인증코드 처리 로직
void DVM::handleAuthCodeFlow() {
    string code = requestAuthCode();

    if (authCodeManager.isValidAuthCode(code)) {
        itemManager.saveSelectedItem(authCodeManager.popAuthCode(code));
        itemManager.showBuyResult();
    } else {
        cout << "해당 인증코드가 존재하지 않습니다." << endl;
    }
}

// 구매 방식 요청 로직
string DVM::askBuyOrCodeInput() {
    string answer;
    while (true) {
        cout << "음료수 구매(1) | 인증코드 입력(2) | 구매 종료(3): ";
        cin >> answer;
        if (answer == "1" || answer == "2" || answer == "3") break;
        cout << "1, 2, 3 중에서 입력해주세요." << endl;
    }
    return answer;
}

// 선결제 의사 확인 로직
string DVM::askUserPrepayment() {
    string answer;
    while (true) {
        cout << "선결제 하시겠습니까? Yes(Y|y) No(N|n): ";
        cin >> answer;
        transform(answer.begin(), answer.end(), answer.begin(), ::tolower);
        if (answer == "y" || answer == "n") break;
        cout << "Y|y 또는 N|n으로 입력해주세요." << endl;
    }
    return answer;
}

// 음료수 선택 요청 로직
pair<int, int> DVM::requestSelect() {
    int itemId, quantity;
    while (true) {
        cout << "음료수 번호를 입력해주세요: ";
        cin >> itemId;

        if (cin.fail() || itemId < 1 || itemId > 20) {
            cin.clear(); // 입력 스트림 오류 플래그 초기화
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 잘못된 입력 무시
            cout << "음료수 번호는 1 ~ 20 사이의 값입니다." << endl;
            continue;
        }
        break;
    }

    while (true) {
        cout << "수량을 입력해주세요: ";
        cin >> quantity;
        if (cin.fail() || quantity < 1 || quantity > 99) {
            cin.clear(); // 입력 스트림 오류 플래그 초기화
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 잘못된 입력 무시
            cout << "음료수 수량은 1~99 까지만 선택 가능합니다." << endl;
            continue;
        }
        break;
    }

    return {itemId, quantity};
}

// 결제 결과 표시 로직
void DVM::showPaymentResult(int payResult) {
    switch (payResult) {
        case 1: cout << "결제 성공하였습니다." << endl; break;
        case 2: cout << "카드 정보가 불일치합니다." << endl; break;
        case 3: cout << "계좌에 돈이 부족합니다." << endl; break;
        default: cout << "알 수 없는 오류가 발생했습니다." << endl; break;
    }
}

// 선결제 결과 표시 로직
void DVM::showPrepaymentResult(const string& authCode, pair<int, int> location) {
    if (authCode != "noprepayment") {
        cout << "인증코드는 " << authCode << "입니다." << endl;
    }

    if (location.first == -1) {
        cout << "대안자판기가 없습니다." << endl;
    } else {
        cout << "대안 자판기의 위치는 " << location.first << ", " << location.second << " 입니다." << endl;
    }
}

// 인증코드 입력 요청 로직
string DVM::requestAuthCode() {
    string code;
    while (true) {
        cout << "인증코드를 입력해주세요(5자리): ";
        cin >> code;
        if (code.length() == 5) break;
        cout << "5자리 인증코드를 입력해주세요." << endl;
    }
    return code;
}