/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofApp.h"

/************************************************************
************************************************************/
#define ERROR_MSG(); printf("Error in %s:%d\n", __FILE__, __LINE__);

void Align_StringOfData(string& s);
void ReadConfig(int& max_points_in_1_frame, int& send_at_once, int& num_log_dirs, float& fps);

/************************************************************
************************************************************/

/******************************
******************************/
int main( int argc, char** argv ){
	/********************
	********************/
	//Use ofGLFWWindowSettings for more options like multi-monitor fullscreen
	ofGLWindowSettings settings;
	settings.setSize(1024, 768);
	settings.windowMode = OF_WINDOW; //can also be OF_FULLSCREEN
	
	auto window = ofCreateWindow(settings);
	
	/********************
	********************/
	int max_points_in_1_frame = 10752;
	int send_at_once = 20;
	int num_log_dirs = 1;
	float fps = 10.0f;
	
	/********************
	********************/
	/*
	printf("---------------------------------\n");
	printf("> parameters\n");
	printf("\t-s      SendAtOnce(20)\n");
	printf("---------------------------------\n");
	
	for(int i = 1; i < argc; i++){
		if( strcmp(argv[i], "-s") == 0 ){
			if(i+1 < argc) { SendAtOnce = atoi(argv[i+1]); i++; }
		}
	}
	*/
	
	ReadConfig(max_points_in_1_frame, send_at_once, num_log_dirs, fps);
	
	/********************
	********************/
	printf("> parameters\n");
	printf("max_points_in_1_frame = %d\n", max_points_in_1_frame);
	printf("send_at_once          = %d\n", send_at_once);
	printf("num_log_dirs          = %d\n", num_log_dirs);
	printf("fps                   = %.2f\n", fps);
	fflush(stdout);
	
	/********************
	********************/
	// ofRunApp(window, make_shared<ofApp>(fps, x_lines, y_lines));
	ofRunApp(window, make_shared<ofApp>(max_points_in_1_frame, send_at_once, num_log_dirs, fps));
	ofRunMainLoop();
}

/******************************
******************************/
void ReadConfig(int& max_points_in_1_frame, int& send_at_once, int& num_log_dirs, float& fps){
	/********************
	********************/
	std::ifstream f_config;
	
	f_config.open("../../../data/Config/Config.txt");
	if(!f_config.is_open())	{ ERROR_MSG(); std::exit(1); }
	
	/********************
	********************/
	while(!f_config.eof()){
		string str_line;
		std::getline(f_config, str_line);
		
		Align_StringOfData(str_line);
		vector<string> block = ofSplitString(str_line, ",");
		
		if(2 <= block.size()){
			if( block[0] == "send_at_once" ){
				send_at_once = atoi(block[1].c_str());
				if( (send_at_once < 20) || (500 < send_at_once) )	{ ERROR_MSG(); std::exit(1); }
			}else if( block[0] == "max_points_in_1_frame" ){
				max_points_in_1_frame = atoi(block[1].c_str());
				if( (max_points_in_1_frame < 10000) || (100000 < max_points_in_1_frame) )	{ ERROR_MSG(); std::exit(1); }
			}else if( block[0] == "num_log_dirs" ){
				num_log_dirs = atoi(block[1].c_str());
				if( (num_log_dirs < 1) || (10 < num_log_dirs) )	{ ERROR_MSG(); std::exit(1); }
			}else if( block[0] == "fps" ){
				fps = (float)atof(block[1].c_str());
				if( (fps < 1.0f) || (100.0f < fps) )	{ ERROR_MSG(); std::exit(1); }
			}
		}
	}
}

/******************************
******************************/
void Align_StringOfData(string& s)
{
	size_t pos;
	while((pos = s.find_first_of(" 　\t\n\r")) != string::npos){ // 半角・全角space, \t 改行 削除
		s.erase(pos, 1);
	}
}
