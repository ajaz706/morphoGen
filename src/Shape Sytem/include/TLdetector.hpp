/*
 * TLdetector.hpp
 * Texture-less object detection class.
 *
 * Example:
 * string tr_bin_file = "tr_data.bin";
 * string test_im_file = "test_01.jpg";
 * CTLdetector detector;
 * detector.train(tr_bin_file,TABLE_GROUPPT);
 * Mat im = imread(test_im_file);
 * detector.detect(im,TABLE_GROUPPT);
 * detector.showDetObjs(im,Scalar(0,255,0),Scalar(255,255,255),640);
 * detector.dispDetObjs();
 *
 *  Created on: Nov 14, 2012
 *      Author: caihongp
 */

#ifndef TLDETECTOR_HPP_
#define TLDETECTOR_HPP_

#undef __GXX_EXPERIMENTAL_CXX0X__
#include <opencv/cv.h>
#include <opencv/highgui.h>
#define BEPRINT   0
#define BETIME    0
#define BETIME_DETAIL 0
#define TABLE_PLAIN   0
#define TABLE_GROUPPT 1
#define TABLE_CLUSTER 2
#define INPUT_CAM 0
#define INPUT_IMG 1
#define INPUT_YARP 2

using namespace cv;
using namespace std;

typedef struct {
	Rect box;
	Rect box_tight;
	int id_label;
	float match_score;
	int id_NN_tr;
	//Mat mat_edge;
	Mat mat_edge_NN_tr;
}DetObj;
typedef struct {
	int h_fix;
	int w_fix;
} ParaNorm;
typedef struct {
	int max_cols_rows;
	float scale_factor;
	float aspect_factor;
	int min_scale;
	int max_scale;
	int min_aspect;
	int max_aspect;
	float canny_tlow;
	float canny_thigh;
	//float canny_sigma;
	float min_edge_pt;
	int nmin_matches;
	int sliding_step;
	float th_overlap_out;
} ParaTest;
typedef struct {
	float th_dis;
	float th_ori;
	float th_match_score;
	//float tola_asp;
	float k_linear;
	float b_linear;
} ParaChamfer;
typedef struct {
	int n_bin_dis;
	int n_bin_ori;
	float MAX_range_dis;
	float MAX_range_ori;
	int n_pt;
	int n_table;
} ParaQuantize;
typedef struct{
	int n_pt;
	int n_table;
	int nmin_matches;
}ParaGroupPt;
typedef struct{
	int n_cell_x;
	int n_cell_y;
	int n_feat_pt;
	vector<Point> inds_feat_pt;
}ParaInds;
typedef struct{
	int n_cls;
	string *all_cls;
}ParaObj;
typedef struct{
	int K;
	float sigma_2;
}ParaCluster;

class CTLdetector {
private:
	vector<DetObj> vec_detobj;
	ParaTest test_para;
	ParaChamfer chamfer_para;
	ParaNorm norm_para;
	ParaQuantize quantize_para;
	ParaGroupPt grouppt_para;
	ParaInds inds_para;
	ParaObj obj_para;
	ParaCluster cluster_para;

	int n_tr;
	bool be_ori_pi;
	vector<int> * ptables;
	Mat mat_inds_grouppt;
	Mat mat_tr_rect_tight;
	vector<int> vec_tr_id_labels;
	vector<int> vec_tr_id_prototype;//used in TABLE_CLUSTER
	vector<int> vec_tr_which_k;     //used in TABLE_CLUSTER
	vector<Mat> vec_mat_tr_edge;
	vector<Mat> vec_mat_tr_ori;
	vector<Mat> vec_mat_tr_dis;

	void generateIndsPt(void);
	int generateTestFeatures(Mat im_cur,
			Mat *out_mat_edge_map,Mat *out_mat_dis_map, Mat *out_mat_ori_map,
			vector<Point> &pout_lu_pts, float **pout_feat_dis,float **pout_feat_ori);
	// generate the 36-pt training features
	void generateTrainFeatures(float *pout_feat_dis, float *pout_feat_ori);
	float autoCannySigma(float ratio);
	void removeLessEdgeWind(Mat im_edge,vector<Point> &pout_lu_pts);
	int refineObj(vector<DetObj> & vec_candi_detobj);
	void drawEdgePatch(Mat &im, Mat &mat_edge_patch, Rect rt, Scalar edge_color);
public:
	CTLdetector();
	void initiate(void);

	bool loadTrainFileBin(string bin_file);
	bool writeTrainFileBin(string bin_file);
	bool loadTrainFileYml(string yml_file);
	bool transferTrainFileYml2Bin(string yml_file, string bin_file);

	bool train(string tr_file,int which_method = 1);
	bool detect(Mat im, int which_method = 1);

	vector<DetObj> getDetObjs(void);
	void showDetObjs(Mat im,Scalar box_color,Scalar edge_color,int w_show_wind=0);
	void dispDetObjs(void);

	~CTLdetector();
};

#endif /* TLDETECTOR_HPP_ */
