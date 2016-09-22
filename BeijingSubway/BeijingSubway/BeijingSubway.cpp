// BeijingSubway.cpp : 定义控制台应用程序的入口点。
//

# include "stdafx.h"
# include <iostream>
# include <fstream>
# include <string>		
# include <array>
# include <vector>
# include <algorithm>
# include <map>
# include <cstdlib>

# define MAX_NUM 1000
using namespace std;


//class Line: 
//store the stations of each line
class Line {
public:
	string name;
	int amount;
	vector<string> stations_name;
	vector<int> stations_id;
	void addStation(string station) { 
		stations_name.push_back(station); 
		amount++;
	}
	void addStation(int id) { 
		stations_id.push_back(id);
	}
	Line(string name) {
		this->name = name;
		amount = 0;
	}
	void printStation() {
		for (vector<string>::iterator iter = stations_name.begin(); iter != stations_name.end(); iter++)
			cout << *iter << endl;
	}
	vector<int> getSubLine(int SourId, int DestId) {
		vector<int> result;
		bool SFind=false, DFind=false, ReverseFlag = false;
		for (int i = 0; i < int(stations_id.size()); i++) {
			if (stations_id[i] == SourId) SFind = true;
			if (stations_id[i] == DestId) DFind = true;
			if (SFind && !DFind) {
				ReverseFlag = false;
				break;
			}
			if (!SFind && DFind) {
				ReverseFlag = true;
				break;
			}
		}
		if (ReverseFlag) {
			int i = stations_id.size()-1;
			while (stations_id[i] != SourId) i--;
			while (stations_id[i] != DestId) {
				result.push_back(stations_id[i]);
				i--;
			}
			result.push_back(DestId);
		}
		else {
			int i = 0;
			while (stations_id[i] != SourId) i++;
			while (stations_id[i] != DestId) {
				result.push_back(stations_id[i]);
				i++;
			}
			result.push_back(DestId);
		}
		return result;

	}
};


//class Station: 
//store the name, line and nearby of a line
//use the name to identify each station
class Station {
public:

	string name;
	int id;
	vector<string> line;
	map<int, string> nearby_id;

	Station(string name) { this->name = name; }
	void addLine(string line) { this->line.push_back(line); }
	void addNearby(int id, string line) { 
		this->nearby_id[id] = line;
	}
	void setId(int id) { this->id = id; }
	bool isNearby(int id) {
		for (map<int, string>::iterator iter = nearby_id.begin(); iter != nearby_id.end(); iter++)
			if (iter->first == id) return true;
		return false;
	}
	void printInfo() {
		cout << name << " ";
		for (vector<string>::iterator iter = line.begin(); iter != line.end(); iter++)
			cout << *iter << endl;
		for (map<int, string>::iterator iter = nearby_id.begin(); iter != nearby_id.end(); iter++)
			cout << iter->first << "(" << iter->second << ")";
	}
};


//Global variable: Lines and Stations
//Store the information subway map in two data structure
vector<Line> Lines;
vector<Station> Stations;
void GetShortestPath(int sour, int dest);
void GetLeastTransferPath(int sour, int dest);
void GetShortestTravesal(int sour);
void formatOutput(vector<int> path);

int main(int argc, char *argv[])
{
	//Reading in map information
	ifstream InFile("beijing-subway.txt");
	if (!InFile) {
		cerr << "file not exist";
		return 0;
	}
	string CurLine, CurStation;
	int NumStation;
	//Build the data strut of Line
	while (InFile >> CurLine >> NumStation) {
		Lines.push_back(Line(CurLine));
		for (int i = 0; i < NumStation; i++) {
			InFile >> CurStation;
			Lines[Lines.size() - 1].addStation(CurStation);
			bool StationExist = false;
			int id;
			for (id=0;id<int(Stations.size());id++) 
				if (Stations[id].name == CurStation) {
					StationExist = true;
					break;
				}
			if (!StationExist) {
				Stations.push_back(Station(CurStation));
				Stations.rbegin()->setId(Stations.size() - 1);
				id = Stations.size() - 1;
			}
			Lines[Lines.size() - 1].addStation(id);
		}
	}
	//Build the nearby relationship of stations
	for (int i = 0; i<int(Lines.size()); i++) {
		for (int j = 0; j < Lines[i].amount; j++) {
			if (j != 0) Stations[Lines[i].stations_id[j]].addNearby(Lines[i].stations_id[j - 1],Lines[i].name);
			if (j != Lines[i].amount - 1) Stations[Lines[i].stations_id[j]].addNearby(Lines[i].stations_id[j + 1], Lines[i].name);
		}
	}
	for (int i = 0; i<int(Stations.size()); i++) {
		for (map<int, string>::iterator iter = Stations[i].nearby_id.begin(); iter != Stations[i].nearby_id.end(); iter++)
			Stations[i].line.push_back(iter->second);
	}


	//Dealing with console command and do some checking
	if (argc == 1) {
		//case1: no argument->loop to wait input then output
		string cmd;
		while (true) {
			cin >> cmd;
			if (cmd == "结束") break;
			else {
				bool exist = false;
				int i;
				for (i = 0; i<int(Lines.size()); i++) {
					if (Lines[i].name == cmd) {
						exist = true;
						break;
					}
				}
				if (exist) Lines[i].printStation();
				else cout << "站点名有误" << endl;
			}
		}
	}
	else if (argc == 4 && string(argv[1]) == "-b") {
		//case2: -b argument->find shortest path and output
		int sour, dest;
		sour = dest = -1;
		for (int i = 0; i<int(Stations.size()); i++) {
			if (Stations[i].name == string(argv[2])) sour = i;
			if (Stations[i].name == string(argv[3])) dest = i;
			if (sour != -1 && dest != -1) break;
		}
		if (sour != -1 && dest != -1) GetShortestPath(sour, dest);
		else cout << "站点输入有误，程序终止运行" << endl;
	}
	else if (argc == 4 && string(argv[1]) == "-c") {
		//case3: -c argument->find "transfer shortest" path and output
		int sour, dest;
		sour = dest = -1;
		for (int i = 0; i<int(Stations.size()); i++) {
			if (Stations[i].name == string(argv[2])) sour = i;
			if (Stations[i].name == string(argv[3])) dest = i;
			if (sour != -1 && dest != -1) break;
		}
		if (sour != -1 && dest != -1) GetLeastTransferPath(sour, dest);
		else cout << "站点输入有误，程序终止运行" << endl;
	}
	else if (argc == 3 && string(argv[1]) == "-a") {
		//case4: -a argument->find shortest traversal
		int sour;
		sour = -1;
		for (int i = 0; i<int(Stations.size()); i++) {
			if (Stations[i].name == string(argv[2])) sour = i;
			if (sour != -1) break;
		}
		if (sour != -1) GetShortestTravesal(sour);
		else cout << "站点输入有误，程序终止运行" << endl;
	}
	else {
		//error case: illegal argument format
		cout << "命令行参数有误，程序终止运行" << endl;
	}





	system("pause");

	return 0;
}

//case2: calculate the shortest path, using dijskra algorithm
void GetShortestPath(int sour, int dest) {
	//dijskra algorithm
	int visited[MAX_NUM],SPL[MAX_NUM],front[MAX_NUM];
	int now;
	for (int i = 0; i<MAX_NUM; i++) {
		visited[i] = 0;
		SPL[i] = MAX_NUM;
		front[i] = 0;
	}
	SPL[sour] = 0;
	now = sour;
	while (visited[dest] == 0) {
		visited[now] = 1;
		for (int i = 0; i<int(Stations.size()); i++)
			if (Stations[now].isNearby(i) && SPL[now] + 1 < SPL[i]) {
				SPL[i] = SPL[now] + 1;
				front[i] = now;
			}
		int min = MAX_NUM;
		for (int i = 0; i<int(Stations.size()); i++)
			if (SPL[i] < min && visited[i] == 0) {
				now = i;
				min = SPL[i];
			}
	}

	//output result
	vector<int> path;
	now = dest;
	path.insert(path.begin(),dest);
	while (now != sour) {
		path.insert(path.begin(),front[now]);
		now = front[now];
	}
	//format outputing
	formatOutput(path);
}

//case3: calculate the shortest "line" path, then calculate the shortest path
void GetLeastTransferPath(int sour, int dest) {
	vector<int> SourLine, DestLine;
	for (int i = 0; i<int(Lines.size()); i++) {
		for (int j = 0; j<int(Stations[sour].line.size()); j++)
			if (Stations[sour].line[j] == Lines[i].name) SourLine.push_back(i);
		for (int j = 0; j<int(Stations[dest].line.size()); j++)
			if (Stations[dest].line[j] == Lines[i].name) DestLine.push_back(i);
	}	

	vector<vector<int> > LineMatrix(MAX_NUM, vector<int>(MAX_NUM));
	for (int i = 0; i < MAX_NUM; i++)
		for (int j = 0; j < MAX_NUM; j++) {
			if (i == j) LineMatrix[i][j] = 0;
			else LineMatrix[i][j] = MAX_NUM;
		}
			
	for (int i = 0; i<int(Lines.size()); i++)
		for (int j = 0; j<int(Lines.size()); j++)
			if (i == j) continue;
			else {
				for (int k = 0; k<int(Lines[i].stations_id.size()); k++)
					for (int l = 0; l<int(Lines[j].stations_id.size()); l++)
						if (Lines[i].stations_id[k] == Lines[j].stations_id[l]) LineMatrix[i][j] = 1;
			}
	//use the Floyd-Warshall Algorithm to find the shortest path
	for (int i = 0; i<int(Lines.size()); i++) {
		for (int j = 0; j<int(Lines.size()); j++) {
			for (int k = 0; k<int(Lines.size()); k++) {
				if (LineMatrix[i][j] > LineMatrix[i][k] + LineMatrix[k][j]) {
					LineMatrix[i][j] = LineMatrix[i][k] + LineMatrix[k][j];
					LineMatrix[j][i] = LineMatrix[i][j];
				}
			}
		}
	}

	//find Line Shortest Path
	//first, find which line to start and which line to end
	int StartLine, ReachLine;
	int MinTransfer = MAX_NUM;
	vector<int> FinalResult;
	for (int i = 0; i<int(SourLine.size()); i++) {
		for (int j = 0; j<int(DestLine.size()); j++) {
			if (LineMatrix[SourLine[i]][DestLine[j]] > MinTransfer) continue;
			else {
				MinTransfer = LineMatrix[SourLine[i]][DestLine[j]];
				StartLine = SourLine[i];
				ReachLine = DestLine[j];

				//try finding the shortest path of the least transfer
				vector<int> LinePath;
				LinePath.insert(LinePath.begin(), StartLine);
				while (ReachLine != StartLine) {
					for (int i = 0; i<int(Lines.size()); i++) {
						if (LineMatrix[i][ReachLine] == LineMatrix[StartLine][ReachLine] - 1 &&
							LineMatrix[i][StartLine] == 1) {
							StartLine = i;
							break;
						}
					}
					LinePath.push_back(StartLine);
				}

				int CurStart, CurEnd;
				CurStart = sour;
				CurEnd = dest; // useless code to pass the code analysis
				vector<int> result, tmp;
				while (true) {
					if (LinePath.size() == 1) CurEnd = dest;
					else {
						//find the transfer station
						for (int i = 0; i < Lines[LinePath[0]].amount; i++)
							for (int j = 0; j < Lines[LinePath[1]].amount; j++) {
								if (Lines[LinePath[0]].stations_id[i] == Lines[LinePath[1]].stations_id[j]) {
									CurEnd = Lines[LinePath[0]].stations_id[i];
									break;
								}
							}
					}
					tmp = Lines[LinePath[0]].getSubLine(CurStart, CurEnd);
					if (LinePath.size() != 1) tmp.pop_back();
					result.insert(result.end(), tmp.begin(), tmp.end());
					CurStart = CurEnd;
					LinePath.erase(LinePath.begin());
					if (LinePath.size() == 0) break;
				}
				
				if (FinalResult.size()==0 || (FinalResult).size() > result.size()) {
					FinalResult.clear();
					FinalResult.insert(FinalResult.begin(), result.begin(), result.end());
				}

			}
		}
	}
	//second, find Line Path
	formatOutput(FinalResult);

	return;
}

//case 4: calculate the shortest travesal starting from a certain station
void GetShortestTravesal(int sour) {

}

void formatOutput(vector<int> path) {
	cout << path.size() << endl;
	for (int i = 0; i<int(path.size()); i++) {
		cout << Stations[path[i]].name;
		if (i != int(path.size()) - 1 && i!=0 && Stations[path[i - 1]].nearby_id[path[i]] != Stations[path[i]].nearby_id[path[i + 1]])
			cout << "换乘" << Stations[path[i]].nearby_id[path[i + 1]];
		cout << endl;
	}
}
