#include <bits/stdc++.h>

#include "jsoncpp/json.h"

using namespace std;

inline void read(string file, Json::Value &v)
{
    static Json::Reader reader;
    static ifstream fin;
    string str;
    fin.open(file);
    getline(fin, str);
    reader.parse(str, v);
    fin.close();
}

inline void write(string file, const Json::Value &v)
{
    static Json::FastWriter writer;
    static ofstream fout;
    fout.open(file);
    fout << writer.write(v);
    fout.close();
}

const string NAME[3] = {"0", "1", "2"};
string PLAYER[3] = {"sample", "sample", "sample"};
Json::Value dat = Json::objectValue;
Json::Value input[3] = {Json::objectValue, Json::objectValue, Json::objectValue};
Json::Value *requests[3] = {
    &(input[0]["requests"] = Json::arrayValue),
    &(input[1]["requests"] = Json::arrayValue),
    &(input[2]["requests"] = Json::arrayValue)
};
Json::Value *responses[3] = {
    &(input[0]["responses"] = Json::arrayValue),
    &(input[1]["responses"] = Json::arrayValue),
    &(input[2]["responses"] = Json::arrayValue)
};

int player_point[3];
int what_to_do;// BIDDING or PLAYING

void init(){
    dat = Json::objectValue;
    dat["log"] = Json::arrayValue;
    for(int i = 0; i < 3; ++i){
	input[i] = Json::objectValue;
	requests[i] = &(input[i]["requests"] = Json::arrayValue);
	responses[i] = &(input[i]["responses"] = Json::arrayValue);
    }
}

int call_judger()
{
//    cout << "call_judger" << endl;
    int nxt = 0;
    write("judgelogs.json", dat);
    system("python3 judge.py >jury_output.json");
    Json::Value &t = dat["log"][dat["log"].size()];
    read("jury_output.json", t);
    if (t.isMember("initdata"))
	dat["initdata"] = t["initdata"];
    if (t["command"] == "request")
    {
	for (int i = 0; i < 3; i++)
	    if (t["content"].isMember(NAME[i])) {
		requests[i]->append(t["content"][NAME[i]]);
		if(t["content"][NAME[i]].isMember("bid"))
		    what_to_do = 0;
		else
		    what_to_do = 1;
		nxt = i;
	    }
    }
    else{
	static int p[3];
	for(int i = 0; i < 3; ++i){
	    p[i] = t["content"][NAME[i]].asInt();
	    player_point[i] += p[i];
	}
	printf("(%d, %d, %d)\n", p[0], p[1], p[2]);
	return -1;
    }
    return nxt;
}

int getid(int c){
    return c < 48 ? c / 4 : std::max(c - 51, 0) + 13;
}
static char *letter = "3456789TJQKA_2mM";


void call_player(int i)
{
    static char Cmd[300];
    write("input.json", input[i]);
    system(("./" + PLAYER[i] + " <input.json >output.json").c_str());
    Json::Value &t = dat["log"][dat["log"].size()][NAME[i]];
    read("output.json", t);
    t["verdict"] = "OK";
    responses[i]->append(t["response"]);
    if(what_to_do == 0){
	printf("PLAYER %d bids:  %d\n", i, t["response"].asInt());
    }
    else{
	printf("PLAYER %d plays: [", i);
	for(unsigned i = 0; i < t["response"].size(); i++)
	    putchar(letter[getid(t["response"][i].asInt())]);
	putchar(']');
	putchar('\n');
    }
}

int main(int argv, char **argc)
{
    if(argv != 4 && argv != 5){
	puts("Invalid Call!");
	exit(0);
    }
    PLAYER[0] = argc[1];
    PLAYER[1] = argc[2];
    PLAYER[2] = argc[3];
    int turns = 1;
    if(argv == 5)
	sscanf(argc[4], "%d", &turns);
    for(int i = 1; i <= turns; ++i){
	printf("GAME %d START!\n", i);
	init();
	call_judger();
	int nxt = 0;
	while (1)
	{
	    call_player(nxt);
	    nxt = call_judger();
	    if(nxt == -1)
		break;
	}
	printf("GAME %d FINISH!\n", i);
	for(int j = 0; j < 3; ++j)
	    printf("player %d has %d point(s) in total.\n", j, player_point[j]);
    }
    return 0;
}
