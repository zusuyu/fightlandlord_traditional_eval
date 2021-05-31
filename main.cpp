#include <iostream>
#include <set>
#include <string>
#include <cassert>
#include <cstring> // 注意memset是cstring里的
#include <algorithm>
#include <fstream>
#include <bitset>
#include <cstdlib>
#include <ctime>
#include "jsoncpp/json.h" // 在平台上，C++编译时默认包含此库

#define PIR std::pair<int, int>


int getid(int c){
    return c < 48 ? c / 4 : std::max(c - 51, 0) + 13;
}

static char *letter = "3456789TJQKA_2mM";

/*
  map: [0, 16) -> {3456789TJQKA_2mM}

  第 12 位被留作空位，是为了确保后面的2和大小王不会作为顺子出现，从而避免繁琐的特判
*/

double get_p() {
    return rand() / double(RAND_MAX);
}
class Combo{
public:
    int k, l, r, w;// [l, r]中的每张牌各k张，带的翼大小为 w
    std::vector<int>wings;
    Combo(){
        k = l = r = w = 0;
        wings = {};
    }
    Combo(int _k, int _l, int _r, int _w){
        k = _k, l = _l, r = _r, w = _w;
        wings = {};
    }
    Combo(int _k, int _l, int _r, int _w, std::vector<int>_wings){
        k = _k, l = _l, r = _r, w = _w;
        wings = _wings;
    }
    Combo(std::vector<int>cards){
        static int cnt[16];
        memset(cnt, 0, sizeof(cnt));
        for(int c: cards)
            ++cnt[getid(c)];
        k = *std::max_element(cnt, cnt + 16);
        for(int i = 0; i < 16; ++i) {
            if(cnt[i] == k){
                l = i;
                while(i + 1 < 16 && cnt[i + 1] == k) ++i;
                r = i;
                break;
            }
        }
        if(k <= 2) // no wings
            w = 0, assert(cards.size() == k * (r - l + 1));
        if(k == 3) // single wing
            w = cards.size() / (r - l + 1) - k;
        if(k == 4) // double wings
            w = (cards.size() / (r - l + 1) - k) / 2;
    }
    int boom_type() {
        if(k == 4 && l == r && w == 0) return l;
        else if(k == 1 && l == 14 && r == 15) return l;
        else return 0;
    }
    bool is_same_pattern(const Combo &b) {
        return k == b.k && r - l == b.r - b.l && w == b.w;
    }
    int card_num() const {
        return k * (r - l + 1) +  w * k;
    }
    int print(){
        int res = 0;
        putchar('[');
        for(int i = l; i <= r; ++i) {
            for(int j = 1; j <= k; ++j) {
                putchar(letter[i]), ++res;
            }
        }
        if(wings.size()) {
            for(int i: wings) {
                for(int j = 1; j <= w; ++j) {
                    putchar(letter[i]), ++res;
                }
            }
        }
        putchar(']');
        putchar(' ');
        return res;
    }
    void turn_to_card(std::vector<int> &possession,std::vector<int> &dest) {
        static int tmp[16];
        memset(tmp,0,sizeof(tmp));
        for(int i = l ; i <= r ; ++i) tmp[i] = k;
        for(auto t : wings) {
            tmp[t] = w;
        }
        dest.clear();
        for(auto c : possession) {
            if(tmp[getid(c)] > 0) {
                --tmp[getid(c)];
                dest.push_back(c);
            }
        }
    }
};

// 以下定义一些全局变量

int MyPosition, LordPosition, LordBid;
std::vector<int>bidInfo;
std::set<int>MyHand, PublicHand;
std::vector<std::vector<int>>GameHistory[3];
int remained_cnt[3] = {17, 17, 17};
Combo lastCombo, going_to_play;
int remained_card[16] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 1, 1};
int what_to_do;// BIDDING or PLAYING
int min_remain_card[6][25];
int max_remain_card[6][25];
int max_remain_cnt;

// 以上是一些全局变量

class Partition{
public:
    std::vector<int>single[4];// 广义单牌，依次表示单张、对子、三连、炸弹
    std::vector<PIR>straight[4];// 广义顺子，依次表示单依次表示单顺、双顺、飞机、航天飞机 pair()标注起止位置（闭）
    int cnt;
    Combo tmp[20];
    Partition(){
        for(int i = 0; i <= 3; ++i){
            single[i].clear();
            straight[i].clear();
        }
    }
    void append(int k, int x){
        single[k - 1].push_back(x);
    }
    void append(int k, int l, int r){
        straight[k - 1].push_back({l, r});
    }
    std::pair<std::vector<int>, std::vector<int>>ccuurr;
    void ddffss(std::vector<int>&vec, int c, int l, int r, int x, std::vector<std::pair<std::vector<int>, std::vector<int>>>&res){
        //	printf("getDaipai-ddffss c = %d, l = %d, r = %d, x = %d\n", c, l, r, x);
        if(x == vec.size()){
            if(c == 0)
		res.push_back(ccuurr);
            return;
        }
        if(c > 0 && (vec[x] < l || vec[x] > r) && (!ccuurr.second.size() || vec[x] != ccuurr.second.back())){
            ccuurr.second.push_back(vec[x]);
            ddffss(vec, c - 1, l, r, x + 1, res);
            ccuurr.second.pop_back();
        }
        ccuurr.first.push_back(vec[x]);
        ddffss(vec, c, l, r, x + 1, res);
        ccuurr.first.pop_back();
    }
    std::vector<std::pair<std::vector<int>, std::vector<int>>>getDaipai(std::vector<int>vec, int c, int l, int r){
        /*
	  printf("getDaipai:\n");
	  for(int x: vec)
	  printf("%d,", x);puts("");
	  printf("c = %d, l = %d, r = %d\n", c, l, r);
        */
        std::vector<std::pair<std::vector<int>, std::vector<int>>> res;
        ccuurr.first.clear();
        ccuurr.second.clear();
        ddffss(vec, c, l, r, 0, res);
        return res;
    }
    std::vector<Combo>cur;
    void dfs(int x, std::vector<std::vector<Combo>>&vec){
        if(x == cnt){
            std::vector<Combo>ccur = cur;
            for(int x: single[0])
		ccur.push_back(Combo(1, x, x, 0));
            for(int x: single[1])
		ccur.push_back(Combo(2, x, x, 0));
            vec.push_back(ccur);
            return;
        }

        // 不带翼
        cur.push_back(Combo(tmp[x].k, tmp[x].l, tmp[x].r, 0));
        dfs(x + 1, vec);
        cur.pop_back();
        // 带单牌还是带对子
        int c = (tmp[x].r - tmp[x].l + 1) * (tmp[x].k == 3 ? 1 : 2);
        for(int w = 0; w < 2; ++w){
            if(single[w].size() < c)
		continue;
            auto sp = getDaipai(single[w], c, tmp[x].l, tmp[x].r);
            std::vector<int>save = single[w];
            for(auto pr: sp){
                assert(save.size() == pr.first.size() + pr.second.size());
                cur.push_back(Combo(tmp[x].k, tmp[x].l, tmp[x].r, w + 1, pr.second));
                single[w] = pr.first;
                dfs(x + 1, vec);
                cur.pop_back();
            }
            single[w] = save;
        }
    }
    void assign_wings(std::vector<std::vector<Combo>>&vec){
        cnt = 0;
        for(int i = 2; i < 4; ++i){
            for(int j: single[i])
		tmp[cnt++] = Combo(i + 1, j, j, 0);
            for(PIR j: straight[i])
		tmp[cnt++] = Combo(i + 1, j.first, j.second, 0);
        }
        for(int i = 0; i < 2; ++i)
	    for(PIR j: straight[i])
		cur.push_back(Combo(i + 1, j.first, j.second, 0));
        dfs(0, vec);
    }

    void info(){
        static char* single_name[4] = {"single", "pair", "triplet", "bomb"};
        static char* straight_name[4] = {"straight", "straight2", "plane", "spaceplane"};
        for(int i = 0; i < 4; ++i)
	    if(single[i].size()){
		printf("%s: ", single_name[i]);
		for(int j: single[i])
		    printf("%c, ", letter[j]);
		puts("");
	    }
        for(int i = 0; i < 4; ++i)
	    if(straight[i].size()){
		printf("%s: ", straight_name[i]);
		for(PIR j: straight[i])
		    printf("(%c,%c), ", letter[j.first], letter[j.second]);
		puts("");
	    }
        puts("");
    }
};

class Hands{
public:
    int cnt[16];
    std::vector<int>card[16];
    std::vector<Partition>Partitions;
    std::vector<std::vector<Combo>>Partitions_with_wings;
    Hands(){
        for(int i = 0; i < 16; ++i){
            cnt[i] = 0;
            card[i].clear();
        }
        Partitions.clear();
    }
    void append(int c){
        int id = getid(c);
        ++cnt[id];
        card[id].push_back(c);
    }
    Hands(std::vector<int>v){
        for(int i = 0; i < 16; ++i){
            cnt[i] = 0;
            card[i].clear();
        }
        Partitions.clear();
        for(int c: v)
	    (*this).append(c);
    }
    std::vector<int> getallcards(){
        std::vector<int> res;
        for(int i = 0; i < 16; ++i)
	    for(int x: card[i])
		res.push_back(x);
        return res;
    }
    std::pair<std::vector<int>, std::vector<int>> split(int *_cnt){
        std::vector<int> a, b;
        for(int i = 0; i < 16; ++i)
	    for(int j = 0; j < cnt[i]; ++j)
		(j < _cnt[i] ? a : b).push_back(card[i][j]);
        return {a, b};
    }
    int tmp[16];
    void dfs2(int x, Partition p){
        if(x == 16){
            Partitions.push_back(p);
            return;
        }
        if(x == 14 && tmp[14] == 1 && tmp[15] == 1){
            Partition q = p;
            q.append(1, 14, 15);
            dfs2(16, q);
        }
        if(tmp[x] == 0)
            dfs2(x + 1, p);
        if(tmp[x] == 1){
            p.append(1, x);
            dfs2(x + 1, p);
        }
        if(tmp[x] == 2){
            Partition q = p;
            q.append(1, x);q.append(1, x);dfs2(x + 1, q);
            q = p;q.append(2, x);dfs2(x + 1, q);
        }
        if(tmp[x] == 3){
            Partition q = p;
            q.append(1, x);q.append(1, x);q.append(1, x);dfs2(x + 1, q);
            q = p;q.append(1, x);q.append(2, x);dfs2(x + 1, q);
            q = p;q.append(3, x);dfs2(x + 1, q);
        }
        if(tmp[x] == 4){
            Partition q = p;
            q.append(1, x);q.append(1, x);q.append(1, x);q.append(1, x);dfs2(x + 1, q);
            q = p;q.append(1, x);q.append(1, x);q.append(2, x);dfs2(x + 1, q);
            q = p;q.append(1, x);q.append(3, x);dfs2(x + 1, q);
            q = p;q.append(2, x);q.append(2, x);dfs2(x + 1, q);
            q = p;q.append(4, x);dfs2(x + 1, q);
        }
    }
    void dfs(int k, int _l, int _r, Partition p){
        /*
	  搜索顺序按航天飞机、飞机、双顺、顺子依次确定广义顺子怎么取，剩下的都是单张，根据假设这些是没有决策的。
	  k \in [1, 4]表示是上面四个的哪一种，_l, _r 是上一个同类型顺子的起止，保证搜出来的状态是唯一的。
        */
        if(k == 0){
            dfs2(0, p);
            return;
        }
        static int min_len[] = {0, 5, 3, 2, 2};
        for(int l = _l; l < 16; ++l){
            int r = l == _l ? _r : l + min_len[k] - 1, valid = 1;
            if(r >= 16)
		continue;
            for(int i = l; i <= r; ++i)
		if(tmp[i] < k)
		    valid = 0;
            if(!valid)
		continue;
            do{
                Partition q = p;
                q.append(k, l, r);
                for(int i = l; i <= r; ++i)
		    tmp[i] -= k;
                dfs(k, l, r, q);
                for(int i = l; i <= r; ++i)
		    tmp[i] += k;
                ++r;
                valid &= tmp[r] >= k;
            }while(r < 16 && valid);
        }
        dfs(k - 1, 0, min_len[k - 1] - 1, p);
    }
    void Try2Partition(){
        memcpy(tmp, cnt, sizeof(tmp));
        Partitions.clear();
        dfs(4, 0, 1, Partition());
        //	printf("total_Partition = %d\n", Partitions.size());
        Partitions_with_wings.clear();
        for(auto x: Partitions){
            //	    x.info();
            x.assign_wings(Partitions_with_wings);
            //	    printf("total_Partitions_with_wings = %d\n", Partitions_with_wings.size());
        }
        //	printf("total_Partitions_with_wings = %d\n", Partitions_with_wings.size());
        /*
	  for(int i = 1; i <= 5; ++i){
	  int id = rand() % Partitions_with_wings.size();
	  int fg = 0;
	  for(Combo c: Partitions_with_wings[id])
	  if(c.k == 3 || c.k == 4)
	  fg = 1;
	  if(fg == 0){
	  --i;
	  continue;
	  }
	  int sum = 0;
	  for(Combo c: Partitions_with_wings[id])
	  sum += c.print();
	  printf("sum = %d\n", sum);
	  }
	*/
    }
    void info(){
        printf("HANDS: [");
        for(int i = 0; i < 16; ++i) {
            for(int j = 0; j < cnt[i]; ++j)
		printf("%c", letter[i]);
        }
        puts("]");
    }
};

void debug_Partition(){
    long long sum1 = 0, sum2 = 0;
    for(int t = 1; t <= 1000; ++t){
        int p[54];
        for(int i = 0; i < 54; ++i){
            p[i] = i;
            std::swap(p[i], p[rand() % (i + 1)]);
        }
        Hands h;
        for(int i = 0; i < 20; ++i)
	    h.append(p[i]);
        h.info();
        h.Try2Partition();
        sum1 += h.Partitions.size();
        sum2 += h.Partitions_with_wings.size();
        if(t % 5 == 0)
	    printf("average Partitions = %.5lf, average Partitions_with_wings = %.5lf\n", 1.0 * sum1 / t, 1.0 * sum2 / t);
    }
}

namespace BotzoneIO{
    using namespace std;
    void read(){
        // 读入输入（平台上的输入是单行）
        string line;

#ifdef ivorysi
        ifstream fin("input.json", ios::in);
        getline(fin, line);
        fin.close();
#else
        getline(cin, line);
#endif
        Json::Value input;
        Json::Reader reader;
        reader.parse(line, input);

        // 首先处理第一回合，得知自己是谁、有哪些牌
        {
            auto firstRequest = input["requests"][0u]; // 下标需要是 unsigned，可以通过在数字后面加u来做到
            auto own = firstRequest["own"];
            for(unsigned i = 0; i < own.size(); i++)
		MyHand.insert(own[i].asInt());
            if(!firstRequest["bid"].isNull()){
                // 如果还可以叫分，则记录叫分
                auto bidHistory = firstRequest["bid"];
                MyPosition = bidHistory.size();
                for(unsigned i = 0; i < bidHistory.size(); i++)
		    bidInfo.push_back(bidHistory[i].asInt());

                what_to_do = 0;// BIDDING
            }
        }

        // history里第一项（上上家）和第二项（上家）分别是谁的决策
        int whoInHistory[] = {(MyPosition + 1) % 3, (MyPosition + 2) % 3};

        int turn = input["requests"].size();
        for(int i = 0; i < turn; i++){
            auto request = input["requests"][i];
            auto llpublic = request["publiccard"];
            if(!llpublic.isNull()){
                // 第一次得知公共牌、地主叫分和地主是谁
                LordPosition = request["landlord"].asInt();
                LordBid = request["finalbid"].asInt();
                MyPosition = request["pos"].asInt();
                remained_cnt[LordPosition] += llpublic.size();

                for(unsigned i = 0; i < llpublic.size(); i++){
                    PublicHand.insert(llpublic[i].asInt());
                    if(LordPosition == MyPosition)
			MyHand.insert(llpublic[i].asInt());
                }
            }

            auto history = request["history"]; // 每个历史中有上家和上上家出的牌
            if(history.isNull())
                continue;
            what_to_do = 1;// PLAYING

            // 逐次恢复局面到当前
            int howManyPass = 0;
            for(int p = 0; p < 2; p++){
                int player = whoInHistory[p];	// 是谁出的牌
                auto playerAction = history[p]; // 出的哪些牌

                vector<int>playerCombo;
                for(unsigned _ = 0; _ < playerAction.size(); _++)
		    playerCombo.push_back(playerAction[_].asInt());

                GameHistory[player].push_back(playerCombo); // 记录游戏历史
                remained_cnt[player] -= playerCombo.size();

                if(playerCombo.size() == 0)
		    ++howManyPass;
                else
		    lastCombo = Combo(playerCombo);
            }

            if(howManyPass == 2)
		lastCombo = Combo();

            if(i < turn - 1){
                // 还要恢复自己曾经出过的牌
                auto playerAction = input["responses"][i]; // 出的哪些牌
                vector<int>MyCombo;
                for(unsigned _ = 0; _ < playerAction.size(); _++){
                    int card = playerAction[_].asInt(); // 这里是自己出的一张牌
                    MyHand.erase(card);				// 从自己手牌中删掉
                    MyCombo.push_back(card);
                }
                GameHistory[MyPosition].push_back(MyCombo); // 记录游戏历史
                remained_cnt[MyPosition] -= MyCombo.size();
            }
        }
        for(int i = 0; i < 3; ++i)
	    for(auto v: GameHistory[i])
		for(int x: v)
		    --remained_card[getid(x)];
        for(int x: MyHand)
            --remained_card[getid(x)];
        /*
	  for(int i = 0; i < 16; ++i)
	  printf("%d", remained_card[i]);
	  puts("");
        */
        //处理一下每种牌型最大值最小值和除了自己以外别人剩余的函数
        {
            for(int i = 1; i <= 4 ; ++i) {
                for(int d = 1 ; d <= 12 ; ++d) {
                    min_remain_card[i][d] = 20;
                    max_remain_card[i][d] = 0;
                    if(i *d > 20) continue;
                    for(int l = 0 ; l < 16 ; ++l) {
                        if(l + d > 16) break;
                        bool f = 1;
                        for(int t = l ; t < l + d ; ++t) {
                            if(remained_cnt[t] < i) {
                                f = 0;break;
                            }
                        }
                        if(f) {
                            min_remain_card[i][d] = min(min_remain_card[i][d],l);
                            max_remain_card[i][d] = max(max_remain_card[i][d],l);
                        }
                    }
                }
            }
            max_remain_cnt = max(remained_cnt[(MyPosition + 1) % 3],remained_cnt[(MyPosition + 2) % 3]);
        }
    }

    /*
      输出叫分（0, 1, 2, 3 四种之一）
    */
    void bid(int value){
        //	printf("bid(%d)\n", value);
        Json::Value result;
        result["response"] = value;
        Json::FastWriter writer;
#ifdef ivorysi
        ofstream fout("output.json");
        fout << writer.write(result) << endl;
        fout.close();
#else
        cout << writer.write(result) << endl;
#endif
    }

    /*
      输出打牌决策
    */
    void play(vector<int> combo_to_play){
        Json::Value result, response(Json::arrayValue);
        for(int c: combo_to_play)
            response.append(c);
        result["response"] = response;
        Json::FastWriter writer;
#ifdef ivorysi
        ofstream fout("output.json");
        fout << writer.write(result) << endl;
        fout.close();
#else
        cout << writer.write(result) << endl;
#endif
    }
}

namespace BIDDING{
    int strategy_0(){
        // 中国有句古话叫...
        return 0;
    }
    int strategy_3(){
        // all in
        return 3;
    }
    int solve(){
        return 0;
    }
}

void print_cards_vector(std::vector<int>cards, char *title = ""){
    std::cerr << title << ": [";
    for(int c: cards)
	   std::cerr<< letter[getid(c)];
    std::cerr << "]" << std::endl;
}
void print_combo(Combo c) {
    std::cerr << "[";
    for(int i = c.l ; i <= c.r ; ++i) {
        for(int j = 1 ; j <= c.k ; ++j) {
            std::cerr << letter[i];
        }
    }
    for(auto t : c.wings) {
        for(int i = 1 ; i <= c.w ; ++i) {
            std::cerr << letter[t];
        }
    }
    std::cerr << "]";
}
void print_combo_vector(std::vector<Combo> pattern) {
    std::cerr << "this pattern is" << std::endl;
    for(auto t : pattern) {
        print_combo(t);
    }
    std::cerr << std::endl;
}
namespace PLAYING{

    int can_follow(Combo a,Combo b) {
        if(!a.boom_type()){
            if(b.boom_type()) return 2;
            else {
                if(a.is_same_pattern(b) && b.l > a.l) return 1;
                else return 0;
            }
        }
        else {
            if(b.boom_type() > a.boom_type()) return 2;
            else return 0;
        }
    }
    double clamp(double x,double min_lim,double max_lim) {
        return std::min(std::max(x,min_lim),max_lim);
    }
    double use_first_hand(Combo &a) {
        if(a.boom_type() > 0) return 0.0;
        if(a.card_num() > max_remain_cnt) return 1.0;

        if(a.k == 4) return 1.0;
        int k = a.k,l = a.r - a.l + 1;
        if(max_remain_card[k][l] < min_remain_card[k][l]) return 1.0;

        double min_val;
        double max_val = 1.0;
        if(k >= 3 && l >= 2) max_val = max_val * l / 1.5;
        if(l >= 7) {
            max_val = max_val * (l  - 5) / 1.5;
        }
        if(k >= 3) min_val = 0.3;
        else min_val = 0.1;

        min_val = min_val * exp(-(max_remain_card[k][l] - min_remain_card[k][l] + 1));

        double res;

        if(a.l <= min_remain_card[k][l]) res = max_val;
        else if(a.l >= max_remain_card[k][l]) res = min_val;
        else if(max_remain_card[k][l] > min_remain_card[k][l]) {
            double a_pow = (min_val - max_val) / pow(max_remain_card[k][l] - min_remain_card[k][l],2);
            res = max_val + a_pow * pow(a.l - min_remain_card[k][l],2);
        }
        else res = max_val;
        return clamp(res,0,1);
    }
    double get_first_hand(Combo &a) {
        if(a.boom_type() > 0) return 1.0;
        if(a.card_num() > max_remain_cnt) return 1.0;
        int k = a.k,l = a.r - a.l + 1;

        if(max_remain_card[k][l] < min_remain_card[k][l]) return 1.0;

        double max_val = 1;
        double min_val = 0;
        if(k == 4) min_val = 0.5;
        else if(k == 3) min_val = 0.3;
        else min_val = 0.0;
        if(k >= 3 && l >= 2) {
            min_val = min_val * l / 1.7;
        }
        if(k == 2 && l >= 3) min_val = 0.22;
        if(l >= 5) {
            min_val = 0.2;
        }
        if(l >= 7) {
            min_val = min_val * (l - 5) / 1.5;
        }

        double res;

        if(a.l >= max_remain_card[k][l]) res = max_val;
        else if(a.l <= min_remain_card[k][l]) res = min_val / (max_remain_card[k][l] - min_remain_card[k][l] + 1);
        else if(max_remain_card[k][l] > min_remain_card[k][l]) {
            double a_pow = (max_val - min_val) / pow(max_remain_card[k][l] - min_remain_card[k][l],2);
            res = min_val + a_pow * pow(a.l - min_remain_card[k][l],2);
        }
        else res = max_val;

        res = clamp(res,0,1);
    }
    double calc_first_hand(Combo lastCombo, std::vector<Combo> partition) {
        double res = 0.0,max_boost = 0.0;
        for(auto p : partition) {
            if(p.boom_type() > 0) continue;
            double tmp = use_first_hand(p);
            res += tmp;
            if(can_follow(lastCombo,p) == 1) max_boost = std::max(max_boost,tmp);
        }
        return res - max_boost;
    }
    double eval(Combo lastCombo, std::vector<Combo> partition) {
        double res = 0.0,max_boost = 0.0;
        for(auto p : partition) {
            if(p.boom_type() > 0) res += 1.0;
            else {
                double tmp = use_first_hand(p);
                res -= tmp;
                res += get_first_hand(p);
                if(can_follow(lastCombo,p) == 1) max_boost = std::max(max_boost,tmp);
            }
        }
        return res + max_boost;
    }
    bool abondon_best(Combo lastCombo, std::vector<Combo> best, std::vector<Combo> can_play_best) {
        double u_best = calc_first_hand(lastCombo, best);
        double u_can_play_best = calc_first_hand(lastCombo, can_play_best);
        if(u_best >= u_can_play_best) return true;
        else {
            if(get_p() < exp(u_can_play_best - u_best)) return true;
            else return false;
        }
    }
    Combo solve(Combo lastCombo, Hands _H){

        Hands H = _H;
        #ifdef zusuyu

        #endif
        H.Try2Partition();
        std::vector<Combo> best;bool can_play = 0;double best_eval = -1e9;
        std::vector<Combo> can_play_best;double can_play_eval = -1e9;
        for(auto pattern : H.Partitions_with_wings) {
            bool is_this_can_play = 0;
            if(lastCombo.k == 0) is_this_can_play = 1;
            else {
                for(auto one_combo : pattern) {
                    int t = can_follow(lastCombo,one_combo);
                    if(t == 1) is_this_can_play = 1;
                }
            }
            double eval_now = eval(lastCombo, pattern);
            if(eval_now > best_eval) {
                best = pattern;
                can_play = is_this_can_play;
                best_eval = eval_now;
            }
            if(is_this_can_play) {
                if(eval_now > can_play_eval) {
                    can_play_best = pattern;
                    can_play_eval = eval_now;
                }
            }
        }
        #ifdef zusuyu
        std::cerr << "best_eval: " <<  best_eval << std::endl;
        std::cerr << "can_play is: " << can_play << std::endl;
        print_combo_vector(best);
        std::cerr << "can_play_eval:" << can_play_eval << std::endl;
        print_combo_vector(can_play_best);
        #endif
        bool can_play_boom = 0;
        if(!can_play) {
            if(best_eval > 0.0 || get_p() < exp(best_eval) + 0.1) {
                for(auto one_combo : best) {
                    if(can_follow(lastCombo,one_combo) == 2) {
                        can_play_boom = 1;break;
                    }
                }
            }
            if(!can_play_boom && can_play_eval > -1e9) {
                if(abondon_best(lastCombo, best, can_play_best)) {
                    can_play = 1;
                    best = can_play_best;
                }
            }
        }

        if(can_play) {
            Combo chu;
            if(lastCombo.k != 0) {
                for(auto one_combo : best) {
                    if(can_follow(lastCombo, one_combo) == 1) {
                        if(chu.k == 0) chu = one_combo;
                        else if(can_follow(one_combo,chu)) chu = one_combo;
                    }
                }
            }
            else {
                double use_first = -1.0,get_first = 0.0;
                for(auto one_combo : best) {
                    double u = use_first_hand(one_combo),g = get_first_hand(one_combo);
                    #ifdef zusuyu
                    print_combo(one_combo);std::cerr << std::endl;
                    std::cerr << "use first hand: " << u << std::endl;
                    std::cerr << "get first hand:" << g << std::endl;
                    #endif
                    if(u > use_first || (u >= use_first - 0.01 && g > get_first) ||
                    (u >= use_first - 0.01 && g >= get_first && one_combo.l < chu.l)) {
                        use_first = u;
                        get_first = g;
                        chu = one_combo;
                        #ifdef zusuyu
                        std::cerr << "change combo to" << std::endl;
                        print_combo(chu);
                        std::cerr << std::endl;
                        #endif
                    }
                }
            }
            return chu;
        }
        else if(can_play_boom) {
            Combo chu;
            for(auto one_combo : best) {
                if(can_follow(lastCombo, one_combo) == 2) {
                    if(chu.k == 0) chu = one_combo;
                    else if(can_follow(one_combo,chu)) chu = one_combo;
                }
            }
            return chu;
        }
        else return Combo();
    }

}

int _main(){
    BotzoneIO::read();
    if(what_to_do == 0){ // BIDDING
        int bidValue = BIDDING::solve();
        BotzoneIO::bid(bidValue);
    }
    else{ // PLAYING
        std::vector<int>curMyHands;

        for(int x: MyHand) {
            curMyHands.push_back(x);

        }
        #ifdef zusuyu
        std::cerr << "Player" << MyPosition << std::endl;
        print_cards_vector(curMyHands);
        #endif
        Combo one_combo = PLAYING::solve(lastCombo, Hands(curMyHands));
        std::vector<int> output;
        one_combo.turn_to_card(curMyHands,output);
        #ifdef zusuyu
        print_cards_vector(output);
        #endif
        BotzoneIO::play(output);
    }
    return 0;
}
/*
  void debug(){
  Hands H;
  int a[] = {3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 10, 11, 12};
  for(int i = 0; i < 15; ++i)
  H.append((a[i] - 3) * 4);
  H.append(52);
  H.append(53);
  PLAYING::solve(Combo(), H);
  }
*/
int main(){
    srand(time(nullptr));
    //debug_Partition();
    _main();
    //    debug();
}
