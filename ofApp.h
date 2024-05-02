/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofxNetwork.h"
#include <ofxGui.h>

/************************************************************
************************************************************/
#define ERROR_MSG(); printf("Error in %s:%d\n", __FILE__, __LINE__);

/************************************************************
************************************************************/

/**************************************************
**************************************************/
class FilesInDir{
private:
	string str_dir_ = "not set yet";
	
	vector<std::ifstream> f_log_;
	vector<string*> file_names_;
	vector<int> frame_id_;
	vector<float> file_fps_;
	
	int file_id_ = 0;
	
	void Align_StringOfData(string& s);
	void DiscardOneLine(std::ifstream& f_log);
	
public:
	~FilesInDir();
	void MakeupFileTable(const string dirname);
	void OpenFiles();
	bool IsEof();
	void FSeekToZero();
	void GetLine(string& str_line);
	void IdNext();
	void IdPrev();
	bool SetId(int id);
	int GetId();
	int GetFrame_Id();
	int GetFileTime();
};

/**************************************************
**************************************************/
struct PointSize{
	float no_sync_;
	float h_;
	float l_;
	
	PointSize(float no_sync, float h, float l)
	: no_sync_(no_sync)
	, h_(h)
	, l_(l)
	{
	}
};

/**************************************************
**************************************************/
class ofApp : public ofBaseApp{
private:
	/****************************************
	****************************************/
	/********************
	********************/
	ofxPanel gui_;
	ofxGuiGroup Group_Limit;
		ofxFloatSlider gui_limit_points_coord_;
		ofxFloatSlider gui_limit_points_to_send_;
	ofxGuiGroup Group_AutoCutChange;
		ofxToggle gui_b_enable_auto_cut_change;
		ofxToggle gui_b_always_start_from_zero;
	ofxGuiGroup Group_misc;
		ofxToggle gui_b_send_udp_dynamic_param;
	
	/********************
	********************/
	const int kUdpBufSize_ = 100000;
	
	ofxUDPManager udp_receive_;
	
	ofxUDPManager udp_send_;
	ofxUDPManager udp_send_point_size_;
	
	bool b_change_contents = false;
	
	/********************
	********************/
	enum{
		kMaxNumDirs_ = 10,
	};
	
	const int kNumDirs_;
	FilesInDir files_in_dir_[kMaxNumDirs_];
	
	int dir_id_ = 0;
	
	const PointSize point_size_[kMaxNumDirs_] = {
		PointSize(0.030, 0.07, 0.03),	// 0
		PointSize(0.035, 0.07, 0.03),	// 1
		PointSize(0.035, 0.07, 0.03),	// 2
		PointSize(0.022, 0.07, 0.03),	// 3
		
		PointSize(0.035, 0.07, 0.03),	// 4
		PointSize(0.035, 0.07, 0.03),	// 5
		PointSize(0.035, 0.07, 0.03),	// 6
		PointSize(0.035, 0.07, 0.03),	// 7
		PointSize(0.035, 0.07, 0.03),	// 8
		PointSize(0.035, 0.07, 0.03),	// 9
	};
	
	const glm::vec3 point_ofs_[kMaxNumDirs_] = {
		glm::vec3(0, 0.43, -2.41), 	// 0
		glm::vec3(0, 0, -3.6), 		// 1
		glm::vec3(0, 0, -5), 		// 2
		glm::vec3(0, 0, -3.6), 		// 3
		
		glm::vec3(0, 0, 0), 	// 4
		glm::vec3(0, 0, 0), 	// 5
		glm::vec3(0, 0, 0), 	// 6
		glm::vec3(0, 0, 0), 	// 7
		glm::vec3(0, 0, 0), 	// 8
		glm::vec3(0, 0, 0), 	// 9
	};
	
	const glm::vec3 rot_deg_[kMaxNumDirs_] = {
		glm::vec3(-27, 180, 0), 	// 0
		glm::vec3(0, 180, 0), 		// 1
		glm::vec3(0, 180, 0), 		// 2
		glm::vec3(0, 180, 0), 		// 3
		
		glm::vec3(0, 180, 0), 	// 4
		glm::vec3(0, 180, 0), 	// 5
		glm::vec3(0, 180, 0), 	// 6
		glm::vec3(0, 180, 0), 	// 7
		glm::vec3(0, 180, 0), 	// 8
		glm::vec3(0, 180, 0), 	// 9
	};
	
	/********************
	********************/
	const int kSendAtOnce_;
	const float kFps_;
	
	int num_points_in_this_frame_;
	int num_points_to_send_;
	int num_packets_;
	
	vector<ofVec3f> positions_;
	
	/********************
	********************/
	// int t_from_ = 0;
	
	int t_elapsed_sec_;
	int t_elapsed_h_;
	int t_elapsed_m_;
	
	/********************
	********************/
	enum class WindowSize{
		kWidth_		= 300,
		kHeight_	= 500,
	};
	
	/********************
	********************/
	enum class State{
		kPause_,
		kPlay_,
	};
	State state_ = State::kPause_;
	
	/********************
	********************/
	ofTrueTypeFont font_M_;
	
	
	/****************************************
	****************************************/
	void SetNextContents();
	void SetupUdp();
	void UpdatePosition();
	void UpdatePosition_with_ReadString(const string& str_line);
	void UpdatePosition_1st();
	void UpdatePosition_kPlay();
	void PrepAndSendUdp();
	void CalElapsedTime();
	void Align_StringOfData(string& s);
	bool CheckIfContentsExist(const string& str_line);
	void TryToReceiveUdpMessage();
	
	static bool CmpVector3ByX(const ofVec3f &a, const ofVec3f &b);
	static bool CmpVector3ByZ(const ofVec3f &a, const ofVec3f &b);
	static bool CmpVector3ByDistance(const ofVec3f &a, const ofVec3f &b);
	
	
public:
	ofApp(int max_points_in_1_frame, int send_at_once, int num_log_dirs, float fps);
	~ofApp();
	
	void setup() override;
	void update() override;
	void draw() override;
	void exit() override;

	void keyPressed(int key) override;
	void keyReleased(int key) override;
	void mouseMoved(int x, int y ) override;
	void mouseDragged(int x, int y, int button) override;
	void mousePressed(int x, int y, int button) override;
	void mouseReleased(int x, int y, int button) override;
	void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
	void mouseEntered(int x, int y) override;
	void mouseExited(int x, int y) override;
	void windowResized(int w, int h) override;
	void dragEvent(ofDragInfo dragInfo) override;
	void gotMessage(ofMessage msg) override;
};
