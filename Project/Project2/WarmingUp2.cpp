#include <iostream>
#include <fstream>
#include <algorithm>
#define NOMINMAX
#include <windows.h>
#include <vector>
#include <cctype>
#include <limits>


using namespace std;

char command;
string sentence;

vector<int> cntWord{};
vector<int> cntUpper{};

vector<int> remeSpace{};

char selectWord{};
char changeWord{};
vector<int> remeWord{};

bool bFlag{ false };
bool cFlag{ false };
bool eFlag{ false };
bool gFlag{ false };
bool hFlag{ false };

bool readText(string title) {
    ifstream file(title);

    if (!file.is_open()) {
        return false;
    }

    sentence.clear();

    char ch;
    bool wasSpace = false;

    while (file.get(ch)) {
        if (ch == ' ') {
            if (wasSpace) {
                continue;
            }
            wasSpace = true;
        }
        else {
            wasSpace = false;
        }
        sentence += ch;
    }

    return true;
}

void reverseReme(int start, int end) {
    if (eFlag && remeSpace.size() == sentence.size()) {
        reverse(remeSpace.begin() + start, remeSpace.begin() + end);
    }

    if (gFlag && remeWord.size() == sentence.size()) {
        reverse(remeWord.begin() + start, remeWord.begin() + end);
    }
}

void chageReme(vector<int>* reme, int lastEnter) {
    vector<int> copy{};

    for (int i = lastEnter + 1; i < reme->size(); ++i) {
        copy.push_back((*reme)[i]);
    }

    copy.push_back((*reme)[lastEnter]);

    for (int i = 0; i < lastEnter; ++i) {
        copy.push_back((*reme)[i]);
    }

    *reme = copy;
}

void countWord(string* sentence) {
    cntWord.clear();
    cntWord.push_back(0);
    
    int lineCnt{};
    bool inWord{ false };

    for (char ch : *sentence) {
        if (ch == '\n') {
            cntWord.push_back(0);
            ++lineCnt;
            inWord = false;
        }
        else if (eFlag ? ch == '*' : ch == ' ') {
            inWord = false;
        }
        else {
            if (!inWord) {
                ++cntWord[lineCnt];
                inWord = true;
            }
        }
    }
}

void countUpper(string* sentence) {
    cntUpper.clear();
    cntUpper.push_back(0);

    int lineCnt{};
    bool firstWord{ true };

    for (char ch : *sentence) {
        if (eFlag ? ch == '*' : ch == ' ') {
            firstWord = true;
        }
        else if (ch == '\n') {
            firstWord = true;
            cntUpper.push_back(0);
            ++lineCnt;
        }
        else {
            if (firstWord) {
                if (isupper(ch)) {
                    ++cntUpper[lineCnt];
                }
            }
            firstWord = false;
        }
    }
}

void printWord(string* sentence) {
    if (bFlag) {
        countWord(sentence);
    }
    else if (cFlag) {
        countUpper(sentence);
    }

    if (bFlag) {
        int lineCnt{ 0 };
        for (char& ch : *sentence) {
            if (ch == '\n') {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
                cout << ' ' << cntWord[lineCnt++] << '\n';
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
            }
            else {
                if (hFlag && isdigit(ch)) {
                    cout << ch << '\n';
                }
                else {
                    cout << ch;
                }
            }
        }
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
        cout << ' ' << cntWord[lineCnt++] << '\n';
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
    }
    else if (cFlag) {
        int lineCnt{ 0 };
        bool firstWord{ true };

        for (char& ch : *sentence) {
            if (ch == '\n') {
                firstWord = true;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
                cout << ' ' << cntUpper[lineCnt++] << '\n';
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
            }
            else if (eFlag ? ch == '*' : ch == ' ') {
                firstWord = true;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);

                if (eFlag) {
                    cout << '*';
                }
                else {
                    cout << ' ';
                }
            }
            else {
                if (firstWord) {
                    if (isupper(ch)) {
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
                    }
                    firstWord = false;
                }

                if (hFlag && isdigit(ch)) {
                    cout << ch << '\n';
                }
                else {
                    cout << ch;
                }
            }
        }

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
        cout << ' ' << cntUpper[lineCnt] << '\n';
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
    }
    else {
        for (char& ch : *sentence) {
            if (hFlag && isdigit(ch)) {
                cout << ch << '\n';
            }
            else {
                cout << ch;
            }
        }
    }
}

void sizeWord(string* sentence) {
    for (char& ch :* sentence) {
        if (islower(ch)) {
            ch=toupper(ch);
        }
        else if (isupper(ch)) {
            ch = tolower(ch);
        }
    }
    printWord(sentence);
}

void numWord(string* sentence) {
    if (cFlag) {
        cFlag = false;
    }

    bFlag = !bFlag;
    printWord(sentence);
}

void colorWord(string* sentence) {
    if (bFlag) {
        bFlag = false;
    }

    cFlag = !cFlag;
    printWord(sentence);
}

void backWord(string* sentence) {
    string rev;
    string org;
    int start{};

    for (int i = 0; i < sentence->size(); ++i) {
        char& ch = (*sentence)[i];

        if (ch == '\n') {
            reverse(rev.begin(), rev.end());
            reverseReme(start, i);

            rev += '\n';
            org += rev;
            rev.clear();

            start = i + 1;
        }
        else {
            rev += ch;
        }
    }

    reverse(rev.begin(), rev.end());
    reverseReme(start, sentence->size());

    org += rev;
    rev.clear();

    *sentence = org;
    printWord(sentence);
}

void spaceWord(string* sentence) {
    int cnt{};

    if (!eFlag) {
        for (char& ch : *sentence) {
            if (ch == ' ') {
                ch = '*';
                remeSpace.push_back(1);
            }
            else {
                remeSpace.push_back(0);
            }
            ++cnt;
        }
    }
    else {
        for (char& ch : *sentence) {
            if (remeSpace[cnt] == 1) {
                if (ch == '*') {
                    ch = ' ';
                }
            }
            ++cnt;
        }
        remeSpace.clear();
    }
    eFlag = !eFlag;
    printWord(sentence);
}

void backLine(string* sentence) {
    string rev;
    string org;
    int start{};

    for (int i = 0; i < sentence->size(); ++i) {
        char& ch = (*sentence)[i];

        if (eFlag ? ch == '*' : ch == ' ') {
            reverse(rev.begin(), rev.end());
            reverseReme(start, i);

            if (eFlag) {
                rev += '*';
            }
            else {
                rev += ' ';
            }

            org += rev;
            rev.clear();

            start = i + 1;
        }
        else if (ch == '\n') {
            reverse(rev.begin(), rev.end());
            reverseReme(start, i);

            rev += '\n';
            org += rev;
            rev.clear();

            start = i + 1;
        }
        else {
            rev += ch;
        }
    }

    reverse(rev.begin(), rev.end());
    reverseReme(start, sentence->size());

    org += rev;
    rev.clear();

    *sentence = org;
    printWord(sentence);
}

void numberLine(string* sentence) {
    hFlag = !hFlag;
    printWord(sentence);
}

void chageWord(string* sentence) {
    gFlag = !gFlag;
    if (gFlag) {
        cout << "어떤 문자를 바꿀까요?\n";
        cin >> selectWord;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "어떤 문자로 바꿀까요?\n";
        cin >> changeWord;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
        for (char& ch : *sentence) {
            if (ch == selectWord) {
                remeWord.push_back(1);
                ch = changeWord;
            }
            else {
                remeWord.push_back(0);
            }
        }
    }
    else {
        int cnt{ 0 };
        for (int num : remeWord) {
            if (num == 1) {
                if ((*sentence)[cnt] == changeWord) {
                    (*sentence)[cnt] = selectWord;
                }
                else if (tolower((*sentence)[cnt]) == tolower(changeWord)) {
                    if (islower(selectWord)) {
                        (*sentence)[cnt] = toupper(selectWord);
                    }
                    else if (isupper(selectWord)) {
                        (*sentence)[cnt] = tolower(selectWord);
                    }
                    else {
                        (*sentence)[cnt] = selectWord;
                    }
                }
            }
            cnt++;
        }
        remeWord.clear();
    }
    printWord(sentence);
}

void findWord(string sentence) {
    string findWord;
    int cnt{};
    int findCnt{};

    cout << "찾을 단어를 입력하세요: ";
    cin >> findWord;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    for (int i = 0; i < sentence.size(); ++i) {

        if (tolower(sentence[i]) == findWord[cnt] || toupper(sentence[i]) == findWord[cnt]) {
            ++cnt;
            if (cnt == findWord.size()) {
                ++findCnt;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);

                for (int j = i - cnt + 1; j <= i; ++j) {
                    cout << sentence[j];
                }

                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
                cnt = 0;
            }
        }
        else {
            if (cnt != 0) {
                cout << sentence[i - cnt];
                i -= cnt;
                cnt = 0;
            }
            else {
                cout << sentence[i];
            }
        }
    }

    for (int i = sentence.size() - cnt; i < sentence.size(); ++i) {
        cout << sentence[i];
    }

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
    cout << "\n찾은 단어 개수: " << findCnt << '\n';
}

void chageLine(string* sentence) {

    if (sentence->empty()) {
        printWord(sentence);
        return;
    }

    string lastLine{};
    string copy{};

    int lastEnter = -1;

    for (int i = sentence->size() - 1; i >= 0; --i) {
        if ((*sentence)[i] == '\n') {
            lastEnter = i;
            break;
        }
    }

    if (lastEnter != -1) {
        for (int i = lastEnter + 1; i < sentence->size(); ++i) {
            lastLine += (*sentence)[i];
        }
        for (int i = 0; i < lastEnter; ++i) {
            copy += (*sentence)[i];
        }
        lastLine += '\n';
        lastLine += copy;

        if (eFlag && remeSpace.size() == sentence->size()) {
            chageReme(&remeSpace, lastEnter);
        }

        if (gFlag && remeWord.size() == sentence->size()) {
            chageReme(&remeWord, lastEnter);
        }


        *sentence = lastLine;
    }
    printWord(sentence);
}

int main() {

    string title;

    while (true) {
        cout << "읽으실 파일을 선택하시오: ";
        cin >> title;

        if (readText(title)) {
            break;
        }
        else {
            cout << "해당 파일이 없습니다.\n";
        }
    }

    printWord(&sentence);

    while (true) {
        cout << "\n\nCommand를 입력하시오: ";

        if (!(cin >> command)) {
            cout << "입력 오류입니다.\n";
            break;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (command == 'a') {
            sizeWord(&sentence);
        }
        else if (command == 'b') {
            numWord(&sentence);
        }
        else if (command == 'c') {
            colorWord(&sentence);
        }
        else if (command == 'd') {
            backWord(&sentence);
        }
        else if (command == 'e') {
            spaceWord(&sentence);
        }
        else if (command == 'f') {
            backLine(&sentence);
        }
        else if (command == 'g') {
            chageWord(&sentence);
        }
        else if (command == 'h') {
            numberLine(&sentence);
        }
        else if (command == 'i') {
            findWord(sentence);
        }
        else if (command == 'j') {
            chageLine(&sentence);
        }
        else if (command == 'q') {
            break;
        }
        else {
            cout << "잘못된 명령어입니다.\n";
        }
    }
}