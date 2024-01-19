#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <tuple>

// 指数分布に基づく次のブロック生成までの時間を計算する関数
double generateBlockTime(double rate, double interval)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<> dist(rate / interval);

    return dist(gen);
}

// シミュレーションのメイン関数
// winFlag，終了時間，ブロック高さ
std::tuple<bool, double, int> simulateDoubleSpendingAttack()
{
    const double alpha = 0.7;        // Honestのハッシュレートの割合
    const double beta = 0.3;         // 攻撃者のハッシュレートの割合
    const int AT = 60 * 60 * 24 * 5; // 攻撃者の制限時間（秒）
    const double T = 60 * 10;        // 平均ブロック生成間隔（秒）
    const int z = 5;                 // 必要なブロック数

    // Honestがゲームをスタートしてから作ったブロック高さ
    int blockHeight = 0;
    // attackerが最終的に作ったブロック数
    int attackerBlockHeight = 0;
    double honestCurrentTime = 0.0;
    double attackerCurrentTime = 0.0;
    bool winFlag = false;
    int finTime = 0;
    std::vector<std::string> blockOrder;
    // winFlag，終了時間，ブロック高さ
    std::tuple<bool, double, int> results;

    // HonestとAttackerが1ブロックずつ生成し，honestCurrentTimeとattackerCurrentTimeを更新していく．
    // 両方のhonestCurrentTimeとattackerCurrentTimeがATを超えたら終了．もしくはHonestがzブロック生成している状態で，attackerCurrentTimeがhonestCurrentTimeより小さければ終了．

    // Honestが1ブロックを先に生成する
    double timeForAttackerBlock = generateBlockTime(beta, T);
    attackerCurrentTime += timeForAttackerBlock;
    attackerBlockHeight++;
    // 終了条件（Attackerの負け）
    if (AT < attackerCurrentTime)
    {
        winFlag = false;
        finTime = AT;
        results = {winFlag, finTime, attackerBlockHeight};
    }
    // 終了条件（Attackerの勝ち）→attackerが1ブロック作って勝つことはない
    // else if (blockHeight >= z - 1 && honestCurrentTime <= attackerCurrentTime)
    // {
    //     winFlag = true;
    //     results = {winFlag, attackerCurrentTime, blockHeight+1};
    // }
    else
    {
        while (true)
        {
            // ブロック高さごとにブロック生成時間の間隔を作成
            double timeForHonestBlock = generateBlockTime(alpha, T);
            double timeForAttackerBlock = generateBlockTime(beta, T);
            honestCurrentTime += timeForHonestBlock;
            attackerCurrentTime += timeForAttackerBlock;
            blockHeight++;

            // 終了条件（Attackerの負け）
            if (AT < attackerCurrentTime)
            {
                winFlag = false;
                finTime = AT;
                attackerBlockHeight = blockHeight + 1;
                results = {winFlag, finTime, attackerBlockHeight};
                break;
            }

            // 終了条件（Attackerの勝ち）
            if (blockHeight >= z - 1 && attackerCurrentTime <= honestCurrentTime)
            {
                winFlag = true;
                finTime = attackerCurrentTime;
                attackerBlockHeight = blockHeight + 1;
                results = {winFlag, finTime, attackerBlockHeight};
                break;
            }
        }
    }

    return results;
}

int main()
{
    // simulateDoubleSpendingAttack()をn=1000回実行して、攻撃者が勝利する確率を求める．
    int n = 100000;
    int attackerWins = 0;
    for (int i = 0; i < n; i++)
    {
        std::tuple<bool, double, int> results = simulateDoubleSpendingAttack();
        if (std::get<0>(results))
        {
            attackerWins++;
        }
    }
    std::cout << "Simulation Counts: " << n << std::endl;
    std::cout << "attackerWins: " << attackerWins << std::endl;
    std::cout << "攻撃者の勝利確率: " << (double)attackerWins / n << std::endl;
    // std::tuple<bool, double, int> results = simulateDoubleSpendingAttack();
    // std::cout << "攻撃者の勝敗" << std::get<0>(results) << ", "
    //           << "ゲーム終了時間" << std::get<1>(results) << ", "
    //           << "攻撃者のブロック高さ" << std::get<2>(results) << std::endl;
    return 0;
}

// Compile: g++ sample.cpp -o sample