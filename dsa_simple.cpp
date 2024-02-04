#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <tuple>
#include <fstream>
#include <sstream>

// 指数分布に基づく次のブロック生成までの時間を計算する関数
double generateBlockTime(double rate, double interval)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<> dist(rate / interval);

    return dist(gen);
}

// シミュレーションのメイン関数
// winFlag，終了時間，ブロック高さ(攻撃者のハッシュレート割合，攻撃者の制限時間)
std::tuple<bool, double, int> simulateDoubleSpendingAttack(double beta, double at)
{
    // 平均ブロック生成間隔（秒）
    const double T = 60 * 10;
    // ファイナリティに必要なブロック数
    const int z = 6;
    // 現在の時間
    double currentTime = 0.0;
    // ゲームが終了した時間
    int finTime = 0;
    // attackerが最終的に作ったブロック数
    int attackerBlockHeight = 0;
    // honestが最終的に作ったブロック数（１ブロックを状態で始まる）
    int honestBlockHeight = 1;
    // 攻撃者が勝利したかどうか
    bool winFlag = false;
    // ブロックを追加した順番のリスト
    // std::vector<std::string> blockOrder;
    // winFlag，終了時間，ブロック高さ
    std::tuple<bool, double, int> results;

    // 勝敗がつくまでループ
    while (true)
    {
        // 次のブロックができる時間を計算
        double timeForBlock = generateBlockTime(1.0, T);
        // 攻撃制限時間を超えたら終了（=攻撃者の負けを判定．ブロックを追加前に終了判定）
        if (currentTime + timeForBlock > at)
        {
            winFlag = false;
            finTime = at;
            results = {winFlag, finTime, attackerBlockHeight};
            break;
        }
        // 確率betaで攻撃者がブロックを生成
        if (beta >= (double)rand() / RAND_MAX)
        {
            currentTime += timeForBlock;
            attackerBlockHeight++;
        }
        else
        {
            currentTime += timeForBlock;
            honestBlockHeight++;
        }
        // 攻撃者の勝利判定（ブロックを追加後に終了判定）
        if (attackerBlockHeight >= z && attackerBlockHeight > honestBlockHeight)
        {
            winFlag = true;
            finTime = currentTime;
            results = {winFlag, finTime, attackerBlockHeight};
            break;
        }
    }

    return results;
}

std::string betaToString(double beta)
{
    std::ostringstream stream;
    stream << beta;
    std::string str = stream.str();
    str.erase(std::remove(str.begin(), str.end(), '.'), str.end());
    return str;
}

int main()
{
    // プログラムの実行時間計測を開始
    auto start = std::chrono::high_resolution_clock::now();

    // simulateDoubleSpendingAttack()をn回実行して、攻撃者が勝利する確率を求める．
    int n = 1000000;
    // 攻撃者のハッシュレートの割合のリスト
    std::vector<double> betaList = {0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45};
    // 攻撃者の制限時間のリスト
    std::vector<int> atList = {60 * 60 * 1, 60 * 60 * 2, 60 * 60 * 3, 60 * 60 * 4, 60 * 60 * 5, 60 * 60 * 6, 60 * 60 * 7, 60 * 60 * 8, 60 * 60 * 9, 60 * 60 * 10, 60 * 60 * 11, 60 * 60 * 12, 60 * 60 * 13, 60 * 60 * 14, 60 * 60 * 15, 60 * 60 * 16, 60 * 60 * 17, 60 * 60 * 18, 60 * 60 * 19, 60 * 60 * 20, 60 * 60 * 21, 60 * 60 * 22, 60 * 60 * 23, 60 * 60 * 24};
    // betaListの要素の数のループを回す
    for (int i = 0; i < betaList.size(); i++)
    {
        // atListの要素の数のループを回す
        for (int j = 0; j < atList.size(); j++)
        {
            // 攻撃者のハッシュレートの割合
            double beta = betaList[i];
            // 攻撃者の制限時間
            double at = atList[j];
            // 攻撃者の勝利回数
            int attackerWins = 0;
            // 攻撃者の勝敗リスト
            std::vector<bool> attackerWinList;
            // 攻撃者の攻撃時間リスト
            std::vector<double> attackerTimeList;
            // 攻撃者のブロック高さリスト
            std::vector<int> attackerBlockHeightList;
            // 攻撃者の勝率のリスト
            std::vector<double> winRateList;
            // n回シミュレーションを実行し，データを配列に格納
            for (int k = 0; k < n; k++)
            {
                std::tuple<bool, double, int> results = simulateDoubleSpendingAttack(beta, at);

                if (std::get<0>(results))
                {
                    attackerWins++;
                }
                winRateList.push_back((double)attackerWins / (k + 1));

                attackerWinList.push_back(std::get<0>(results));
                attackerTimeList.push_back(std::get<1>(results));
                attackerBlockHeightList.push_back(std::get<2>(results));
            }

            // winRateList，attackerWinList，attackerTimeList，attackerBlockHeightListを1つのCSVファイルに出力
            std::ofstream file("1m_simple/b" + betaToString(beta) + "_at" + std::to_string(static_cast<int>(at / 60)) + ".csv");
            for (int l = 0; l < attackerWinList.size(); l++)
            {
                file << winRateList[l] << "," << attackerWinList[l] << "," << attackerTimeList[l] << "," << attackerBlockHeightList[l] << std::endl; // ここでCSV形式に合わせてデータを出力
            }
            file.close();
        }
    }

    // 時間計測を終了
    auto end = std::chrono::high_resolution_clock::now();

    // 経過時間を計算
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // 経過時間を出力（単位は秒で）
    std::cout << "実行時間: " << duration / 1000000.0 << "秒" << std::endl;

    return 0;
}

// Compile: g++ dsa.cpp -o dsa_ex