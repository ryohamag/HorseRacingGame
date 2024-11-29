#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NUM_OF_UPPER_NAME 101
#define NUM_OF_LOWER_NAME 100
#define NUM_OF_JOCKEY 105

struct horseData
{
    int number; //馬番
    char name[64]; //名前
    double appLevel; //支持率
    char gender[4]; //性別
    int age; //年齢
    char jockey[32]; //ジョッキー
};

//複勝のオッズを返却するための構造体
struct PlaceShowOdds
{
    double minOdds; //最小値
    double maxOdds; //最大値
    double odds; //確定値
};

// 名前の生成
void getName(char *name, char UpperNames[][NUM_OF_UPPER_NAME], char LowerNames[][NUM_OF_LOWER_NAME]) {
    char upper[32];
    char lower[32];
    strcpy(upper, UpperNames[rand() % NUM_OF_UPPER_NAME]);
    strcpy(lower, LowerNames[rand() % NUM_OF_LOWER_NAME]);
    snprintf(name, 64, "%s%s\t", upper, lower);
}

//人気順に並び替える
void sortByPopularity(struct horseData *horseData, int numOfHorse) {
    for (int i = 0; i < numOfHorse - 1; i++) {
        for (int j = numOfHorse - 1; j > i; j--) {
            if (horseData[j - 1].appLevel < horseData[j].appLevel) {
                struct horseData tmp = horseData[j - 1];
                horseData[j - 1] = horseData[j];
                horseData[j] = tmp;
            }
        }
    }
}

// 単勝オッズの計算
double CalcWinOdds(struct horseData *horseData) {
    double odds = 0.8 / horseData->appLevel;
    if (odds >= 1000) {
        odds = 999.9;
    }
    return odds;
}

//複勝オッズの計算
struct PlaceShowOdds CalcPlaceShowOdds(struct horseData *horseData, struct horseData *selectedHorse,  int numOfHorse, int condition) {
    double otherAppLevel = 0.0; //外れ馬の支持率の合計
    struct PlaceShowOdds odds = {0.0, 0.0, 0.0}; //複勝オッズの最大値と最小値、または確定オッズ

    if (condition == 0) { //出馬表用
        sortByPopularity(horseData, numOfHorse);

        //選択された馬が1or2番人気であるかそうでないかで場合分け
        if (selectedHorse->number != horseData[0].number && selectedHorse->number != horseData[1].number) {
            for (int i = 2; i < numOfHorse; i++) {
                if (selectedHorse->number == horseData[i].number) {
                    continue;
                } else {
                    otherAppLevel += horseData[i].appLevel;
                }
            }
            odds.minOdds = (selectedHorse->appLevel + otherAppLevel / 3) * 0.8 / selectedHorse->appLevel; //複勝オッズの最小値
        } else if (selectedHorse->number == horseData[0].number || selectedHorse->number == horseData[1].number) {
            for (int i = 3; i < numOfHorse; i++) {
                otherAppLevel += horseData[i].appLevel;
            }
            odds.minOdds = (selectedHorse->appLevel + otherAppLevel / 3) * 0.8 / selectedHorse->appLevel;
        }

        //選択された馬が最下位人気またはブービー人気であるかそうでないかで場合分け
        if (selectedHorse->number != horseData[numOfHorse - 2].number && selectedHorse->number != horseData[numOfHorse - 1].number) {
            for (int i = 0; i < numOfHorse - 2; i++) {
                if (selectedHorse->number == horseData[i].number) {
                    continue;
                } else {
                    otherAppLevel += horseData[i].appLevel;
                }
            }
            odds.maxOdds = (selectedHorse->appLevel + otherAppLevel / 3) * 0.8 / selectedHorse->appLevel; //複勝オッズの最大値
        } else if (selectedHorse->number == horseData[numOfHorse - 2].number || selectedHorse->number == horseData[numOfHorse - 1].number) {
            for (int i = 0; i < numOfHorse - 2; i++) {
                otherAppLevel += horseData[i].appLevel;
            }
            odds.maxOdds = (selectedHorse->appLevel + otherAppLevel / 3) * 0.8 / selectedHorse->appLevel;
        }
        return odds;
    } else if (condition == 1) { //結果用
        for (int i = 3; i < numOfHorse; i++) {
            otherAppLevel += horseData[i].appLevel;
        }
        odds.odds = (selectedHorse->appLevel + otherAppLevel / 3) * 0.8 / selectedHorse->appLevel;
        return odds;
    }
}

//馬連のオッズの計算
double **CalcQuinellaOdds(struct horseData *horseData, int numOfHorse) {
    double **odds = malloc(numOfHorse * sizeof(double *));
    for (int i = 0; i < numOfHorse; i++) {
        odds[i] = malloc(numOfHorse * sizeof(double));
    }

    srand(time(NULL));
    for (int i = 0; i < numOfHorse; i++) {
        for (int j = i + 1; j < numOfHorse; j++) {
            odds[i][j] = ((CalcWinOdds(&horseData[i]) + CalcWinOdds(&horseData[j])) / 2.0) * (rand() % 4 + 1);
            if (odds[i][j] >= 1000) {
                odds[i][j] = 999.9;
            }
        }
    }
    return odds;
}

// 出馬表の表示
void printRaceCard(struct horseData *horseData, int numOfHorse) {
    printf("人気順オッズ\n");
    printf("馬番|馬名               |単勝 |複勝       |性齢|騎手\n");
    for (int i = 0; i < numOfHorse; i++) {
        struct PlaceShowOdds placeShow = CalcPlaceShowOdds(horseData, &horseData[i], numOfHorse, 0);
        double winOdds = CalcWinOdds(&horseData[i])*10;
        double minOdds = placeShow.minOdds*10;
        double maxOdds = placeShow.maxOdds*10;
        printf("%4d|%-16s|%5.1f|%5.1f-%5.1f|%s%2d|%-16s\n", horseData[i].number, horseData[i].name, floor(winOdds)/10, floor(minOdds)/10, floor(maxOdds)/10, horseData[i].gender, horseData[i].age, horseData[i].jockey);
    }
}

//馬連のオッズの表示
void printQuinellaOdds(double **quinellaOdds, int numOfHorse) {
    printf("馬連オッズ\n");

    // 列のヘッダーを表示
    for (int i = 0; i < numOfHorse - 1; i++) {
        printf("%8d|", i + 1);
    }
    printf("\n");

    for (int i = 0; i < numOfHorse - 1; i++) {
        for (int j = 0; j < numOfHorse - i - 1; j++) {
            printf("%2d:%5.1f|", i + j + 2, quinellaOdds[j][i + j + 1]);
        }
        printf("\n");
    }
}

//着順の決定
void decideOrder(struct horseData *horseData, int numOfHorse) {
    struct horseData *order = malloc(sizeof(struct horseData) * numOfHorse);

    sortByPopularity(horseData, numOfHorse);

    srand(time(NULL));
    int WinningNumber = rand() % 100 + 1; //1から100の乱数を生成
    int count = 0;
    int selected[numOfHorse]; // 選ばれた馬のインデックスを追跡
    for (int i = 0; i < numOfHorse; i++) {
        selected[i] = 0; // 初期化
    }

    int i = 0;
    do {
        for (int j = 0; j < horseData[i % numOfHorse].appLevel * 100; j++) {
            int num = rand() % 100 + 1;
            if (WinningNumber == num && !selected[i % numOfHorse]) {
                order[count] = horseData[i % numOfHorse];
                selected[i % numOfHorse] = 1; // 選ばれた馬をマーク
                count++;
                break;
            }
        }
        i++;
    } while (count != numOfHorse);

    // horseDataを着順通りに書き換える
    for (int i = 0; i < numOfHorse; i++) {
        horseData[i] = order[i];
    }

    free(order);
}

//組み合わせの計算
int combination(int n, int r) {
    if (r == 0 || r == n) {
        return 1;
    } else {
        return combination(n - 1, r - 1) + combination(n - 1, r);
    }
}


//メイン関数
int main() {
    // 名前データの読み込み
    FILE *fp;
    char UpperNames[NUM_OF_UPPER_NAME][NUM_OF_UPPER_NAME];
    int lineCount = 0;

    // UpperName.txtを開く
    fp = fopen("UpperName.txt", "r");
    if (fp == NULL) {
        printf("\aファイルが開けません。\n");
        return 0;
    }

    // ファイルから一行ずつ読み込む
    while (lineCount < NUM_OF_UPPER_NAME && fgets(UpperNames[lineCount], sizeof(UpperNames[lineCount]), fp) != NULL) {
        // 改行を削除
        UpperNames[lineCount][strcspn(UpperNames[lineCount], "\n")] = '\0';
        lineCount++;
    }

    fclose(fp);

    char LowerNames[NUM_OF_LOWER_NAME][NUM_OF_LOWER_NAME];
    lineCount = 0;

    // LowerName.txtを開く
    fp = fopen("LowerName.txt", "r");
    if (fp == NULL) {
        printf("\aファイルが開けません。\n");
        return 0;
    }

    while (lineCount < NUM_OF_LOWER_NAME && fgets(LowerNames[lineCount], sizeof(LowerNames[lineCount]), fp) != NULL) {
        LowerNames[lineCount][strcspn(LowerNames[lineCount], "\n")] = '\0';
        lineCount++;
    }

    fclose(fp);

    char JockeyNames[NUM_OF_JOCKEY][NUM_OF_JOCKEY];
    lineCount = 0;

    // Jockey.txtを開く
    fp = fopen("Jockey.txt", "r");
    if (fp == NULL) {
        printf("\aファイルが開けません。\n");
        return 0;
    }

    while (lineCount < NUM_OF_JOCKEY && fgets(JockeyNames[lineCount], sizeof(JockeyNames[lineCount]), fp) != NULL) {
        JockeyNames[lineCount][strcspn(JockeyNames[lineCount], "\n")] = '\0';
        lineCount++;
    }

    fclose(fp);

    srand(time(NULL)); //乱数の種を初期化

    //出走頭数
    int numOfHorse = rand() % 11 + 8; //8から18の乱数を生成

    int jockeySelected[NUM_OF_JOCKEY] = {0};

    //データの生成
    struct horseData horseData[numOfHorse];
    double totalAppLevel = 0.0;

    for (int i = 0; i < numOfHorse; i++) {
        horseData[i].number = i + 1;
        getName(horseData[i].name, UpperNames, LowerNames);
        horseData[i].appLevel = (double)rand() / RAND_MAX; // ランダムな支持率を生成
        totalAppLevel += horseData[i].appLevel;
        strcpy(horseData[i].gender, rand() % 2 == 0 ? "牡" : "牝");
        horseData[i].age = rand() % 8 + 3;
        int jockeyIndex;
        do {
            jockeyIndex = rand() % NUM_OF_JOCKEY;
        } while (jockeySelected[jockeyIndex]);
        jockeySelected[jockeyIndex] = 1;
        strcpy(horseData[i].jockey, JockeyNames[jockeyIndex]);
    }

    // 支持率を正規化して合計が1になるようにする
    for (int i = 0; i < numOfHorse; i++) {
        horseData[i].appLevel /= totalAppLevel;
    }

    double **quinellaOdds = CalcQuinellaOdds(horseData, numOfHorse); //馬連オッズ

    printRaceCard(horseData, numOfHorse); //出馬表の表示

    printQuinellaOdds(quinellaOdds, numOfHorse); //馬連オッズの表示

    int WinBetNumber[numOfHorse]; //賭けた単勝の馬番の配列
    int WinBetAmount[numOfHorse]; //賭けた単勝の金額の配列
    int PlaceBetNumber[numOfHorse]; //賭けた複勝の馬番の配列
    int PlaceBetAmount[numOfHorse]; //賭けた複勝の金額の配列
    int QuinellaBetNumber1[combination(numOfHorse,2)]; //賭けた馬連の1頭目の馬番の配列
    int QuinellaBetNumber2[combination(numOfHorse,2)]; //賭けた馬連の2頭目の馬番の配列
    int QuinellaBetAmount[combination(numOfHorse,2)]; //賭けた馬連の金額の配列

    int betType; //券種
    int totalBetAmount = 0; //総賭け金
    int continueFlag = 1; //続けるかどうかのフラグ
    int winCount = 0; //単勝の賭け数
    int placeCount = 0; //複勝の賭け数
    int quinellaCount = 0; //馬連の賭け数

    do {
    printf("式別を選択\n");
    printf("1:単勝, 2:複勝, 3:馬連\n");
    scanf("%d", &betType);
    if (betType < 1 || betType > 3) {
        printf("1から3の値を入力してください。\n");
        continue;
    }
    if (betType == 1) {
        printf("馬番を1つ選択\n");
        int betNumber; //馬番
        int betAmount; //金額
        printf("馬番(1~%d):", numOfHorse);
        scanf("%d", &betNumber);
        if (betNumber < 1 || betNumber > numOfHorse) {
            printf("1から%dの値を入力してください。\n", numOfHorse);
            continue;
        }
        WinBetNumber[winCount] = betNumber;
        printf("金額(100円単位):");
        scanf("%d", &betAmount);
        if (betAmount < 100 || betAmount > __INT_MAX__ || betAmount % 100 != 0) {
            printf("100円単位での値を入力してください。\n");
            WinBetNumber[winCount] = 0;
            continue;
        }
        totalBetAmount += betAmount;
        WinBetAmount[winCount] = betAmount;
        winCount++;
    } else if (betType == 2) {
        printf("馬番を1つ選択\n");
        int betNumber; //馬番
        printf("馬番(1~%d):", numOfHorse);
        scanf("%d", &betNumber);
        if (betNumber < 1 || betNumber > numOfHorse) {
            printf("1から%dの値を入力してください。\n", numOfHorse);
            continue;
        }
        PlaceBetNumber[placeCount] = betNumber;
        printf("金額(100円単位):");
        int betAmount; //金額
        scanf("%d", &betAmount);
        if (betAmount < 100 || betAmount > __INT_MAX__ || betAmount % 100 != 0) {
            printf("100円単位での値を入力してください。\n");
            PlaceBetNumber[placeCount] = 0;
            continue;
        }
        totalBetAmount += betAmount;
        PlaceBetAmount[placeCount] = betAmount;
        placeCount++;
    } else if (betType == 3) {
        int type = 0; //通常、ながし、ボックスの選択
        printf("投票方法\n");
        while(1) {
            printf("1:通常, 2:ながし, 3:ボックス\n");
            scanf("%d", &type);
            if (type < 1 || type > 3) {
                printf("1から3の値を入力してください。\n");
                continue;
            }
            break;
        }

        if (type == 1) {
            printf("馬番を2つ選択\n");
            int betNumber1, betNumber2; //馬番
            while(1) {
                printf("一頭目(1~%d):", numOfHorse);
                scanf("%d", &betNumber1);
                if (betNumber1 < 1 || betNumber1 > numOfHorse) {
                    printf("1から%dの値を入力してください。\n", numOfHorse);
                    continue;
                }
                break;
            }
            while(1) {
                printf("二頭目(1~%d):", numOfHorse);
                scanf("%d", &betNumber2);
                if (betNumber2 < 1 || betNumber2 > numOfHorse) {
                    printf("1から%dの値を入力してください。\n", numOfHorse);
                    continue;
                }
                if (betNumber1 == betNumber2) {
                    printf("同じ馬を選択することはできません。\n");
                    continue;
                }
                break;
            }
            if (betNumber1 < betNumber2) {
                QuinellaBetNumber1[quinellaCount] = betNumber1;
                QuinellaBetNumber2[quinellaCount] = betNumber2;
            } else {
                QuinellaBetNumber1[quinellaCount] = betNumber2;
                QuinellaBetNumber2[quinellaCount] = betNumber1;
            }
            int betAmount; //金額
            while(1) {
                printf("金額(100円単位):");
                scanf("%d", &betAmount);
                if (betAmount < 100 || betAmount > __INT_MAX__ || betAmount % 100 != 0) {
                    printf("100円単位での値を入力してください。\n");
                    QuinellaBetNumber1[quinellaCount] = 0;
                    QuinellaBetNumber2[quinellaCount] = 0;
                    continue;
                }
                break;
            }
            totalBetAmount += betAmount;
            QuinellaBetAmount[quinellaCount] = betAmount;
            quinellaCount++;
        } else if (type == 2) {
            printf("軸馬を1つ選択\n");
            int favoriteNumber; //軸馬の馬番
            printf("軸馬(1~%d):", numOfHorse);
            scanf("%d", &favoriteNumber);
            if (favoriteNumber < 1 || favoriteNumber > numOfHorse) {
                printf("1から%dの値を入力してください。\n", numOfHorse);
                continue;
            }
            int longShotCount = 0; //相手の数
            printf("相手を1頭ずつ入力(終わる場合は0を入力)\n");
            while(1) {
                int longShotNumber; //相手の馬番
                printf("相手(1~%d):", numOfHorse);
                scanf("%d", &longShotNumber);
                if (longShotCount == 0 & longShotNumber == 0) {
                    printf("1頭以上の馬を選択してください。\n");
                    continue;
                }
                if (longShotNumber < 0 || longShotNumber > numOfHorse) {
                    printf("1から%dの値を入力してください。\n", numOfHorse);
                    continue;
                }
                if (favoriteNumber == longShotNumber) {
                    printf("同じ馬を選択することはできません。\n");
                    continue;
                }
                if (longShotNumber == 0) {
                    break;
                }
                if (favoriteNumber < longShotNumber) {
                    QuinellaBetNumber1[quinellaCount] = favoriteNumber;
                    QuinellaBetNumber2[quinellaCount] = longShotNumber;
                } else {
                    QuinellaBetNumber1[quinellaCount] = longShotNumber;
                    QuinellaBetNumber2[quinellaCount] = favoriteNumber;
                }
                longShotCount++;
                quinellaCount++;
            }
            printf("金額(100円単位):");
            int betAmount; //金額
            while(1) {
                scanf("%d", &betAmount);
                if (betAmount < 100 || betAmount > __INT_MAX__ || betAmount % 100 != 0) {
                    printf("100円単位での値を入力してください。\n");
                    continue;
                }
                break;
            }
            for (int i = 0; i < longShotCount; i++) {
                totalBetAmount += betAmount;
                QuinellaBetAmount[quinellaCount - longShotCount + i] = betAmount;
            }
        }
    }
    printf("投票を続ける場合は1を入力。1以外を入力で出走します。\n");
    scanf("%d", &continueFlag);
    printf("\n");
    } while (continueFlag == 1);

    decideOrder(horseData, numOfHorse); //着順の決定

    //結果の表示
    printf("結果\n");
    printf("着順  馬番  馬名                 騎手\n");
    for (int i = 0; i < numOfHorse; i++) {
        printf("%2d着 %4d   %-16s %-16s\n", i + 1, horseData[i].number, horseData[i].name, horseData[i].jockey);
    }
    printf("\n");

    printf("払戻金\n");
    printf("単勝\n");
    double odds = floor(CalcWinOdds(&horseData[0]) * 100)/10;
    printf("%4d:%5d円\n", horseData[0].number, (int)odds*10);


    printf("複勝\n");
    for (int i = 0; i < 3; i++) {
        struct PlaceShowOdds placeShow = CalcPlaceShowOdds(horseData, &horseData[i], numOfHorse, 1);
        double odds = floor(placeShow.odds * 100)/10;
        printf("%4d:%5d円\n", horseData[i].number, (int)odds*10);
    }

    printf("馬連\n");
    if (horseData[0].number < horseData[1].number) {
        double odds = floor(quinellaOdds[horseData[0].number - 1][horseData[1].number - 1]*100)/10;
        printf("%2d-%2d:%5d円\n", horseData[0].number, horseData[1].number, (int)odds*10);
    } else {
        double odds = floor(quinellaOdds[horseData[1].number - 1][horseData[0].number - 1]*100)/10;
        printf("%d-%d:%5d円\n", horseData[1].number, horseData[0].number, (int)odds*10);
    }

    printf("\n");

    //買った馬券の払い戻し
    int totalWin = 0;
    //単勝の払い戻し
    if (winCount != 0) {
        for (int i = 0; i < winCount; i++) {
            if (WinBetNumber[i] == horseData[0].number) {
                double odds = floor(CalcWinOdds(&horseData[0]) * 100)/10;
                int win = ((int)odds*10)*(WinBetAmount[i]/100);
                totalWin += win;
                printf("単勝的中:%5d円\n", win);
            }
        }
    }

    //複勝の払い戻し
    if (placeCount != 0) {
        for (int i = 0; i < placeCount; i++) {
            for (int j = 0; j < 3; j++) {
                if (PlaceBetNumber[i] == horseData[j].number) {
                    struct PlaceShowOdds placeShow = CalcPlaceShowOdds(horseData, &horseData[j], numOfHorse, 1);
                    double odds = floor(placeShow.odds * 100)/10;
                    int win = ((int)odds*10)*(PlaceBetAmount[i]/100);
                    totalWin += win;
                    printf("複勝的中:%5d円\n", win);
                }
            }
        }
    }

    //馬連の払い戻し
    if (quinellaCount != 0) {
        for (int i = 0; i < quinellaCount; i++) {
            if (QuinellaBetNumber1[i] == horseData[0].number & QuinellaBetNumber2[i] == horseData[1].number) {
                double odds = floor(quinellaOdds[horseData[0].number - 1][horseData[1].number - 1] * 100)/10;
                int win = ((int)odds*10)*(QuinellaBetAmount[i]/100);
                totalWin += win;
                printf("馬連的中:%5d円\n", win);
            } else if (QuinellaBetNumber1[i] == horseData[1].number & QuinellaBetNumber2[i] == horseData[0].number) {
                double odds = floor(quinellaOdds[horseData[1].number - 1][horseData[0].number - 1] * 100)/10;
                int win = ((int)odds*10)*(QuinellaBetAmount[i]/100);
                totalWin += win;
                printf("馬連的中:%5d円\n", win);
            }
        }
    }
    

    printf("総投資額:%5d円\n", totalBetAmount);
    printf("合計払戻金:%5d円\n", totalWin);
    printf("回収率:%5.1f%%\n", (double)totalWin / totalBetAmount * 100);

    // メモリの解放
    for (int i = 0; i < numOfHorse; i++) {
        free(quinellaOdds[i]);
    }
    free(quinellaOdds);

    return 0;
}