#include <iostream>
#include <set>
#include <string>
#include <cassert>
#include <cstring> // 注意memset是cstring里的
#include <algorithm>
#include <fstream>
#include <bitset>
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
	for(int i = 0; i < 16; ++i)
	    if(cnt[i] == k){
		l = i;
		while(i + 1 < 16 && cnt[i + 1] == k)
		    ++i;
		r = i;
		break;
	    }
	if(k <= 2) // no wings
	    w = 0, assert(cards.size() == k * (r - l + 1));
	if(k == 3) // single wing
	    w = cards.size() / (r - l + 1) - k;
	if(k == 4) // double wings
	    w = (cards.size() / (r - l + 1) - k) / 2;
    }
    int print(){
	int res = 0;
	putchar('[');
	for(int i = l; i <= r; ++i)
	    for(int j = 1; j <= k; ++j)
		putchar(letter[i]), ++res;
	if(wings.size())
	    for(int i: wings)
		for(int j = 1; j <= w; ++j)
		    putchar(letter[i]), ++res;
	putchar(']');
	putchar(' ');
	return res;
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
	for(int i = 0; i < 16; ++i)
	    for(int j = 0; j < cnt[i]; ++j)
		printf("%c", letter[i]);
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
	int res = strategy_0();
	return res;
    }
}

void print_cards_vector(std::vector<int>cards, char *title = ""){
    printf("%s: [", title);
    for(int c: cards)
	putchar(letter[getid(c)]);
    puts("]");
}

namespace PLAYING{
    std::vector<int> solve(Combo lastCombo, Hands _H){
    }
}
/*
int _main(){
    BotzoneIO::read();
    if(what_to_do == 0){ // BIDDING
	int bidValue = BIDDING::solve();
	BotzoneIO::bid(bidValue);
    }
    else{ // PLAYING
	std::vector<int>curMyHands;
	for(int x: MyHand)
	    curMyHands.push_back(x);
	std::vector<int>output = PLAYING::solve(lastCombo, Hands(curMyHands));
	BotzoneIO::play(output);
    }
    return 0;
}

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
    debug_Partition();
//    _main();
//    debug();
}

