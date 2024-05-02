/************************************************************
************************************************************/
#include "ofApp.h"
#include <algorithm> // to use std::min

/* for dir search */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <dirent.h>
#include <string>

// using namespace std;

/************************************************************
************************************************************/

/******************************
******************************/
FilesInDir::~FilesInDir(){
	for(int i = 0; i < (int)f_log_.size(); i++){
		if(f_log_[i].is_open()) f_log_[i].close();
	}
	
	for(int i = 0; i < file_names_.size(); i++){
		delete file_names_[i];
	}
	
	printf("> FilesInDir %s : dtr() \n", str_dir_.c_str());
	fflush(stdout);
}

/******************************
******************************/
void FilesInDir::MakeupFileTable(const string dirname)
{
	/********************
	********************/
	str_dir_ = dirname;
	printf("> search csv Files in %s\n", str_dir_.c_str());
	
	/********************
	********************/
	DIR *pDir;
	struct dirent *pEnt;
	struct stat wStat;
	string wPathName;

	pDir = opendir( str_dir_.c_str() );
	if ( NULL == pDir ) { ERROR_MSG(); std::exit(1); }
	
	pEnt = readdir( pDir );
	while ( pEnt ) {
		// .と..は処理しない
		if ( strcmp( pEnt->d_name, "." ) && strcmp( pEnt->d_name, ".." ) ) {
		
			wPathName = str_dir_ + "/" + pEnt->d_name;
			
			// ファイルの情報を取得
			if ( stat( wPathName.c_str(), &wStat ) ) {
				printf( "Failed to get stat %s \n", wPathName.c_str() );
				break;
			}
			
			if ( S_ISDIR( wStat.st_mode ) ) {
				// nothing.
			} else {
				vector<string> str = ofSplitString(pEnt->d_name, ".");
				if(str[str.size()-1] == "csv"){
					// string str_NewFileName = wPathName;
					// string str_NewFileName = pEnt->d_name;
					// string* str_NewFileName = new string(pEnt->d_name);	// data dir 直下にmp3置いている場合はこちらでOK
					string* str_NewFileName = new string(wPathName);		// 違う場合はFullPath.
					
					printf("%s\n", pEnt->d_name);
					
					file_names_.push_back(str_NewFileName);
				}
			}
		}
		
		pEnt = readdir( pDir ); // 次のファイルを検索する
	}

	closedir( pDir );
	
	/********************
	********************/
	if(file_names_.size() == 0){
		char buf[100];
		printf("> no files in %s\n", str_dir_.c_str());
		fflush(stdout);
		std::exit(1);
	}
 }

/******************************
******************************/
void FilesInDir::OpenFiles()
{
	/********************
	********************/
	printf("> open csv files in %s\n", str_dir_.c_str());
	
	/********************
	********************/
	f_log_.resize(file_names_.size());
	
	frame_id_.resize(file_names_.size());
	for(int i = 0; i < (int)frame_id_.size(); i++)	{ frame_id_[i] = -1; }
	
	file_fps_.resize(file_names_.size());
	
	/********************
	********************/
	for(int i = 0; i < (int)f_log_.size(); i++){
		/********************
		********************/
		f_log_[i].open(file_names_[i]->c_str());
		if(!f_log_[i].is_open()){
			printf("> file open error : %s\n", file_names_[i]->c_str());
			std::exit(1);
		}else{
			printf("%3d:%s\n", i, file_names_[i]->c_str());
		}
		
		/********************
		********************/
		string str_line;
		std::getline(f_log_[i], str_line);
		Align_StringOfData(str_line);
		
		vector<string> block = ofSplitString(str_line, ",");
		if( (block.size() < 2) || (block[0] != "fps") ) { ERROR_MSG(); std::exit(1); }
		
		float fps = atof(block[1].c_str());
		if( (fps < 5) || (60 < fps) ) { ERROR_MSG(); std::exit(1); }
		file_fps_[i] = fps;
		printf("\tfps = %f\n", fps);
		
		/********************
		********************/
		if(f_log_[i].eof()) { ERROR_MSG(); std::exit(1); }
	}
	printf("--------------------\n");
	fflush(stdout);
}


/******************************
******************************/
void FilesInDir::DiscardOneLine(std::ifstream& f_log){
	string str_line;
	std::getline(f_log, str_line);
}

/******************************
******************************/
void FilesInDir::Align_StringOfData(string& s)
{
	size_t pos;
	while((pos = s.find_first_of(" 　\t\n\r")) != string::npos){ // 半角・全角space, \t 改行 削除
		s.erase(pos, 1);
	}
}

/******************************
******************************/
bool FilesInDir::IsEof()
{
	return f_log_[file_id_].eof();
}

/******************************
******************************/
void FilesInDir::FSeekToZero(){
	/********************
	■C++におけるrewindのお作法
		https://sleepy-yoshi.hatenablog.com/entry/20120508/p1
	********************/
	f_log_[file_id_].clear();
	f_log_[file_id_].seekg(0, std::ios::beg);
	frame_id_[file_id_] = -1;
	
	DiscardOneLine(f_log_[file_id_]);
	
	if(f_log_[file_id_].eof()) { ERROR_MSG(); std::exit(1); } // means : no data
}

/******************************
******************************/
void FilesInDir::GetLine(string& str_line){
	if(f_log_[file_id_].eof()){
		str_line = "";
	}else{
		std::getline(f_log_[file_id_], str_line);
		Align_StringOfData(str_line);
		frame_id_[file_id_]++;
	}
}

/******************************
******************************/
void FilesInDir::IdNext(){
	file_id_++;
	if(f_log_.size() <= file_id_) file_id_ = 0;
}

/******************************
******************************/
void FilesInDir::IdPrev(){
	file_id_--;
	if(file_id_ < 0) file_id_ = f_log_.size() - 1;
}

/******************************
******************************/
int FilesInDir::GetId(){
	return file_id_;
}

/******************************
******************************/
bool FilesInDir::SetId(int id){
	if( (0 <= id) && (id < f_log_.size()) ){
		file_id_ = id;
		return true;
	}else{
		return false;
	}
}

/******************************
******************************/
int FilesInDir::GetFrame_Id(){
	return frame_id_[file_id_];
}

/******************************
******************************/
int FilesInDir::GetFileTime(){
	return (int)(frame_id_[file_id_] * 1000.0f / file_fps_[file_id_]);
}

/************************************************************
************************************************************/

/******************************
******************************/
ofApp::ofApp(int max_points_in_1_frame, int send_at_once, int num_log_dirs, float fps)
: kSendAtOnce_(send_at_once)
, kFps_(fps)
, kNumDirs_( (num_log_dirs <= kMaxNumDirs_) ? num_log_dirs : kMaxNumDirs_ )
, num_points_in_this_frame_(0)
{
	/********************
	********************/
	font_M_.load("font/RictyDiminished-Regular.ttf", 18/* font size in pixels */, true/* _bAntiAliased */, true/* _bFullCharacterSet */, false/* makeContours */, 0.3f/* simplifyAmt */, 72/* dpi */);
	
	/********************
	********************/
	positions_.resize(max_points_in_1_frame);
	
	/********************
	********************/
	for(int i = 0; i < kNumDirs_; i++){
		char buf[100];
		snprintf(buf, std::size(buf), "../../../data/Log/Log_%d", i);
		files_in_dir_[i].MakeupFileTable(buf);
		files_in_dir_[i].OpenFiles();
	}
}

/******************************
******************************/
ofApp::~ofApp(){
}

/******************************
******************************/
void ofApp::setup(){
	/********************
	********************/
	ofSetWindowTitle("LiDz:LogSender");
	
	ofSetWindowShape( static_cast<int>(WindowSize::kWidth_), static_cast<int>(WindowSize::kHeight_) );
	ofSetVerticalSync(false);	// trueとどっちがいいんだろう？
	ofSetFrameRate(kFps_);
	
	ofSetEscapeQuitsApp(false);
	
	/********************
	********************/
	ofEnableAntiAliasing();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
	
	/********************
	********************/
	SetupUdp();
	
	/********************
	********************/
	gui_.setup("param", "gui.xml", 47, 182);
	
	Group_Limit.setup("Limit");
		Group_Limit.add(gui_limit_points_coord_.setup("limit_points_coord_", 0.0, 0.0, 2.0));
		Group_Limit.add(gui_limit_points_to_send_.setup("limit_points_to_send_", 42500, 0.0, 57600));
	gui_.add(&Group_Limit);
	
	Group_AutoCutChange.setup("AutoCutChange");
		Group_AutoCutChange.add(gui_b_enable_auto_cut_change.setup("enable_auto_cut_change", true));
		Group_AutoCutChange.add(gui_b_always_start_from_zero.setup("always_start_from_zero", false));
	gui_.add(&Group_AutoCutChange);
	
	Group_misc.setup("misc");
		Group_misc.add(gui_b_send_udp_dynamic_param.setup("send_udp_dynamic_param", true));
	gui_.add(&Group_misc);
	
	/********************
	********************/
	UpdatePosition_1st();
}

/******************************
******************************/
void ofApp::SetupUdp(){
	/********************
	********************/
	{
		ofxUDPSettings settings;
		settings.sendTo("127.0.0.1", 12347);
		settings.blocking = false;
		
		udp_send_.Setup(settings);
	}
	
	{ // udp_receive_ : 通常時のみ
		ofxUDPSettings settings;
		settings.receiveOn(12348);
		settings.blocking = false;
		
		udp_receive_.Setup(settings);
	}
	
	{
		ofxUDPSettings settings;
		settings.sendTo("127.0.0.1", 12349);
		settings.blocking = false;
		
		udp_send_point_size_.Setup(settings);
	}
}

/******************************
******************************/
void ofApp::TryToReceiveUdpMessage(){
	/********************
	********************/
	char udp_message[kUdpBufSize_];
	
	udp_receive_.Receive(udp_message, kUdpBufSize_);
	string message = udp_message;
	
	/********************
	********************/
	while(message!=""){
		vector<string> str_messages = ofSplitString(message, "[/p]");
		
		if(str_messages[0] == "/SoundSyncCutChange"){
			b_change_contents = true;
		}
		
		/********************
		********************/
		udp_receive_.Receive(udp_message, kUdpBufSize_);
		message = udp_message;
	}
}

/******************************
******************************/
void ofApp::SetNextContents(){
	files_in_dir_[dir_id_].IdNext();
	
	dir_id_++;
	if(kNumDirs_ <= dir_id_) dir_id_ = 0;
	
	// t_from_ = ofGetElapsedTimeMillis();
	
	if(gui_b_always_start_from_zero) files_in_dir_[dir_id_].FSeekToZero();
}

/******************************
******************************/
void ofApp::update(){
	TryToReceiveUdpMessage();
	
	if(state_ == State::kPlay_){
		if(gui_b_enable_auto_cut_change && b_change_contents) SetNextContents();
		
		UpdatePosition_kPlay();
		PrepAndSendUdp();
		CalElapsedTime();
	}
	
	b_change_contents = false;
}

/******************************
******************************/
void ofApp::UpdatePosition(){
	/********************
	********************/
	int c_loop_contents = 0;
	string str_line;
	do{
		if(files_in_dir_[dir_id_].IsEof()){
			files_in_dir_[dir_id_].FSeekToZero();
			c_loop_contents++;
			if(2 <= c_loop_contents)	{ ERROR_MSG(); std::exit(1); }
		}
		
		files_in_dir_[dir_id_].GetLine(str_line);
	}while( !CheckIfContentsExist(str_line) );
	
	/********************
	********************/
	vector<string> block = ofSplitString(str_line, ",");
	num_points_in_this_frame_ = (int)block.size() / 3;
	if( ((int)block.size() < 3) || ((int)block.size() % 3 != 0) || ((int)positions_.size() < (int)block.size()/3) ) { ERROR_MSG(); std::exit(1); }
	
	/********************
	********************/
	for(int i = 0; i < positions_.size(); i++){
		float x, y, z;
		if( i < num_points_in_this_frame_ ){
			x = atof(block[i * 3 + 0].c_str());
			y = atof(block[i * 3 + 1].c_str());
			z = atof(block[i * 3 + 2].c_str());
			
		}else{
			float far = 10000;
			x = far;
			y = far;
			z = far;
		}
		
		positions_[i].set(x, y, z);
	}
	
	/********************
	********************/
	switch((int)gui_limit_points_coord_){
		case 0:
			std::sort(positions_.begin(), positions_.end(), CmpVector3ByX); // 比較関数cmpを使用してsort
			break;
			
		case 1:
			std::sort(positions_.begin(), positions_.end(), CmpVector3ByZ); // 比較関数cmpを使用してsort
			break;
			
		case 2:
			std::sort(positions_.begin(), positions_.end(), CmpVector3ByDistance); // 比較関数cmpを使用してsort
			break;
	}
	
	
	/********************
	********************/
	num_points_to_send_ = std::min( num_points_in_this_frame_, (int)gui_limit_points_to_send_ );
	
	/********************
	********************/
	num_packets_ = num_points_to_send_ / kSendAtOnce_;
	if(num_points_to_send_ % kSendAtOnce_ != 0) num_packets_++;
}

/******************************
******************************/
void ofApp::UpdatePosition_with_ReadString(const string& str_line){
	/********************
	********************/
	vector<string> block = ofSplitString(str_line, ",");
	num_points_in_this_frame_ = (int)block.size() / 3;
	if( ((int)block.size() < 3) || ((int)block.size() % 3 != 0) || ((int)positions_.size() < (int)block.size()/3) ) { ERROR_MSG(); std::exit(1); }
	
	/********************
	********************/
	for(int i = 0; i < positions_.size(); i++){
		float x, y, z;
		if( i < num_points_in_this_frame_ ){
			x = atof(block[i * 3 + 0].c_str());
			y = atof(block[i * 3 + 1].c_str());
			z = atof(block[i * 3 + 2].c_str());
			
		}else{
			float far = 10000;
			x = far;
			y = far;
			z = far;
		}
		
		positions_[i].set(x, y, z);
	}
	
	/********************
	********************/
	switch((int)gui_limit_points_coord_){
		case 0:
			std::sort(positions_.begin(), positions_.end(), CmpVector3ByX); // 比較関数cmpを使用してsort
			break;
			
		case 1:
			std::sort(positions_.begin(), positions_.end(), CmpVector3ByZ); // 比較関数cmpを使用してsort
			break;
			
		case 2:
			std::sort(positions_.begin(), positions_.end(), CmpVector3ByDistance); // 比較関数cmpを使用してsort
			break;
	}
	
	/********************
	********************/
	num_points_to_send_ = std::min( num_points_in_this_frame_, (int)gui_limit_points_to_send_ );
	
	/********************
	********************/
	num_packets_ = num_points_to_send_ / kSendAtOnce_;
	if(num_points_to_send_ % kSendAtOnce_ != 0) num_packets_++;
}

/******************************
******************************/
void ofApp::UpdatePosition_1st(){
	string str_line;
	
	files_in_dir_[dir_id_].GetLine(str_line);
	if( !CheckIfContentsExist(str_line) )	{ ERROR_MSG(); std::exit(1); }
	
	UpdatePosition_with_ReadString(str_line);
}

/******************************
******************************/
void ofApp::UpdatePosition_kPlay(){
	/********************
	********************/
	int c_loop_contents = 0;
	string str_line;
	do{
		if(files_in_dir_[dir_id_].IsEof()){
			files_in_dir_[dir_id_].FSeekToZero();
			c_loop_contents++;
			if(2 <= c_loop_contents)	{ ERROR_MSG(); std::exit(1); }
		}
		
		files_in_dir_[dir_id_].GetLine(str_line);
	}while( !CheckIfContentsExist(str_line) );
	
	UpdatePosition_with_ReadString(str_line);
}

/******************************
******************************/
bool ofApp::CmpVector3ByX(const ofVec3f &a, const ofVec3f &b)
{
	return std::abs(a.x) < std::abs(b.x);	// 昇順
}

/******************************
******************************/
bool ofApp::CmpVector3ByZ(const ofVec3f &a, const ofVec3f &b)
{
	return std::abs(a.z) < std::abs(b.z);	// 昇順
}

/******************************
******************************/
bool ofApp::CmpVector3ByDistance(const ofVec3f &a, const ofVec3f &b)
{
	// return std::abs(a.x * a.x + a.z * a.z) < std::abs(b.x * b.x + b.z * b.z);	// 昇順
	return a.squareDistance( ofVec3f(0, 0, 0) ) < b.squareDistance( ofVec3f(0, 0, 0) ); // 昇順
}

/******************************
******************************/
void ofApp::Align_StringOfData(string& s)
{
	size_t pos;
	while((pos = s.find_first_of(" 　\t\n\r")) != string::npos){ // 半角・全角space, \t 改行 削除
		s.erase(pos, 1);
	}
}

/******************************
description
	str_line must be aligned before call.
******************************/
bool ofApp::CheckIfContentsExist(const string& str_line)
{
	vector<string> block = ofSplitString(str_line, ",");
	if( (block.size() == 0) || (block[0] == "") ){ // no_data or exist text but it's",,,,,,,".
		return false;
	}else{
		return true;
	}
}

/******************************
******************************/
void ofApp::PrepAndSendUdp(){
	const int kBufSize = 100;
	
	for(int ofs = 0, id = 0; ofs < num_points_to_send_; ofs += kSendAtOnce_, id++){
		int count = std::min(kSendAtOnce_, num_points_to_send_ - ofs); // positions_.size() は unsigned なので、castしないと、build通らない
		
		string message = "/pos,";
		if(gui_b_send_udp_dynamic_param) message = "/pos_and_dynamic_param,";
		
		{
			char buf[kBufSize];
			snprintf(buf, std::size(buf), "%d,%d,%d,%d,%d,", 1/* grop_id */, num_points_to_send_, num_packets_, id, ofs);
			
			message += buf;
		}
		
		if(gui_b_send_udp_dynamic_param){
			char buf[kBufSize];
			snprintf(buf, std::size(buf), "%.3f,%.3f,%.3f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,", point_size_[dir_id_].no_sync_, point_size_[dir_id_].h_, point_size_[dir_id_].l_, point_ofs_[dir_id_].x, point_ofs_[dir_id_].y, point_ofs_[dir_id_].z, rot_deg_[dir_id_].x, rot_deg_[dir_id_].y, rot_deg_[dir_id_].z);
			
			message += buf;
		}
		
		for(int i = 0; i < count; i++){
			char buf[kBufSize];
			
			snprintf(buf, std::size(buf), "%.2f,%.2f,%.2f", positions_[i + ofs].x, positions_[i + ofs].y, positions_[i + ofs].z );
			
			message += buf;
			
			if(i < count - 1)	{ message += ","; }
		}
		
		udp_send_.Send(message.c_str(), message.length());
	}
}

/******************************
******************************/
void ofApp::CalElapsedTime(){
	// t_elapsed_sec_	= (ofGetElapsedTimeMillis() - t_from_) / 1000;
	t_elapsed_sec_	= files_in_dir_[dir_id_].GetFileTime() / 1000;
	
	t_elapsed_h_	=  t_elapsed_sec_ / (60 * 60);
	t_elapsed_sec_	-= t_elapsed_h_ * (60 * 60);
	
	t_elapsed_m_	=  t_elapsed_sec_ / 60;
	t_elapsed_sec_	-= t_elapsed_m_ * 60;
}


/******************************
******************************/
void ofApp::draw(){
	/********************
	********************/
	ofBackground(30);
	
	/********************
	********************/
	ofSetColor(255);
	
	char buf[500];
	
	switch(state_){
		case State::kPause_:
			font_M_.drawString("Pause", 47, 45);
			
			snprintf(buf, std::size(buf), "dir id       : %02d", dir_id_);
			font_M_.drawString(buf, 47, 80);
			
			snprintf(buf, std::size(buf), "file  id     : %02d", files_in_dir_[dir_id_].GetId());
			font_M_.drawString(buf, 47, 105);
			
			snprintf(buf, std::size(buf), "frame id     : %08d", files_in_dir_[dir_id_].GetFrame_Id());
			font_M_.drawString(buf, 47, 130);
			
			snprintf(buf, std::size(buf), "will send at : %02d fps", (int)ofGetFrameRate());
			font_M_.drawString(buf, 47, 165);
			
			break;
			
		case State::kPlay_:
			font_M_.drawString("Play", 47, 45);
			
			snprintf(buf, std::size(buf), "%02d:%02d:%02d", t_elapsed_h_, t_elapsed_m_, t_elapsed_sec_);
			font_M_.drawString(buf, 181, 45);
			
			snprintf(buf, std::size(buf), "dir id       : %02d", dir_id_);
			font_M_.drawString(buf, 47, 80);
			
			snprintf(buf, std::size(buf), "file  id     : %02d", files_in_dir_[dir_id_].GetId());
			font_M_.drawString(buf, 47, 105);
			
			snprintf(buf, std::size(buf), "frame id     : %08d", files_in_dir_[dir_id_].GetFrame_Id());
			font_M_.drawString(buf, 47, 130);
			
			snprintf(buf, std::size(buf), "sending at   : %02d fps", (int)ofGetFrameRate());
			font_M_.drawString(buf, 47, 165);
			break;
	}
	
	/********************
	********************/
	gui_.draw();
}

/******************************
******************************/
void ofApp::exit(){

}

/******************************
******************************/
void ofApp::keyPressed(int key){
	switch(key){
		case ' ':
			if(state_ == State::kPause_){
				state_ = State::kPlay_;
			}else{
				state_ = State::kPause_;
			}
			break;
		
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			int file_id = key - '0';
			
			if(files_in_dir_[dir_id_].SetId(file_id)){
				// t_from_ = ofGetElapsedTimeMillis();
				if(gui_b_always_start_from_zero) files_in_dir_[dir_id_].FSeekToZero();
			}
		}
			break;
		
		case OF_KEY_UP:
			files_in_dir_[dir_id_].IdPrev();
			if(gui_b_always_start_from_zero) files_in_dir_[dir_id_].FSeekToZero();
			break;
			
		case OF_KEY_DOWN:
			files_in_dir_[dir_id_].IdNext();
			if(gui_b_always_start_from_zero) files_in_dir_[dir_id_].FSeekToZero();
			break;
			
			
		case OF_KEY_RIGHT:
			dir_id_++;
			if(kNumDirs_ <= dir_id_) dir_id_ = 0;
			// t_from_ = ofGetElapsedTimeMillis();
			if(gui_b_always_start_from_zero) files_in_dir_[dir_id_].FSeekToZero();
			break;
			
		case OF_KEY_LEFT:
			dir_id_--;
			if(dir_id_ < 0) dir_id_ = kNumDirs_ - 1;
			// t_from_ = ofGetElapsedTimeMillis();
			if(gui_b_always_start_from_zero) files_in_dir_[dir_id_].FSeekToZero();
			break;
	}
}

/******************************
******************************/
void ofApp::keyReleased(int key){

}

/******************************
******************************/
void ofApp::mouseMoved(int x, int y ){

}

/******************************
******************************/
void ofApp::mouseDragged(int x, int y, int button){

}

/******************************
******************************/
void ofApp::mousePressed(int x, int y, int button){

}

/******************************
******************************/
void ofApp::mouseReleased(int x, int y, int button){

}

/******************************
******************************/
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

/******************************
******************************/
void ofApp::mouseEntered(int x, int y){

}

/******************************
******************************/
void ofApp::mouseExited(int x, int y){

}

/******************************
******************************/
void ofApp::windowResized(int w, int h){

}

/******************************
******************************/
void ofApp::gotMessage(ofMessage msg){

}

/******************************
******************************/
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
