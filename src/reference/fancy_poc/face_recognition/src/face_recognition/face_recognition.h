/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _FACE_RECOGNITION_H
#define _FACE_RECOGNITION_H

#include <vector>
#include "utils.h"
#include "ai_base.h"

using std::vector;

typedef struct FaceRecognitionInfo
{
    int id;                     //人脸识别结果对应ID
    float score;                //人脸识别结果对应得分
    string name;                //人脸识别结果对应人名
} FaceRecognitionInfo;

/**
 * @brief 基于Retinaface的人脸检测
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class FaceRecognition : public AIBase
{
public:
    /**
     * @brief FaceRecognition构造函数，加载kmodel,并初始化kmodel输入、输出(for image)
     * @param kmodel_file       kmodel文件路径
     * @param max_register_face 数据库最多可以存放的人脸特征数
     * @param thresh            人脸识别阈值
     * @param debug_mode        0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceRecognition(const char *kmodel_file, int max_register_face, float thresh, const int debug_mode);

    /**
     * @brief FaceRecognition构造函数，加载kmodel,并初始化kmodel输入、输出和人脸检测阈值(for isp)
     * @param kmodel_file       kmodel文件路径
     * @param max_register_face 数据库最多可以存放的人脸特征数
     * @param thresh            人脸识别阈值
     * @param isp_shape         isp输入大小（chw）
     * @param vaddr             isp对应虚拟地址
     * @param paddr             isp对应物理地址
     * @param debug_mode        0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceRecognition(const char *kmodel_file,int max_register_face, float thresh, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode);

    /**
     * @brief FaceRecognition析构函数
     * @return None
     */
    ~FaceRecognition();

    /**
     * @brief 图片预处理        （ai2d for image）
     * @param ori_img          原始图片
     * @param sparse_points    原始人脸检测框对应的五官点
     * @return None
     */
    void pre_process(cv::Mat ori_img, float* sparse_points);

    /**
     * @brief 视频流预处理（ai2d for video）
     * @param sparse_points    原始人脸检测框对应的五官点
     * @return None
     */
    void pre_process(float* sparse_points);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    //for database
    /**
     * @brief 人脸数据库加载接口
     * @param db_pth 数据库目录
     * @return None
     */
    // void database_init();
    void database_init(char *db_pth);

    /**
     * @brief 人脸数据库查询接口
     * @param result 人脸识别结果
     * @return None
     */
    void database_search(FaceRecognitionInfo& result);

    /**
     * @brief 将处理好的轮廓画到原图
     * @param src_img     原图
     * @param bbox        识别人脸的检测框位置
     * @param result      人脸识别结果
     * @param pic_mode    ture(原图片)，false(osd)
     * @return None
     */
    void draw_result(cv::Mat& src_img,Bbox& bbox,FaceRecognitionInfo& result, bool pic_mode=true);

private:
    /** 
     * @brief svd
     * @param a     原始矩阵
     * @param u     左奇异向量
     * @param s     对角阵
     * @param v     右奇异向量
     * @return None
     */
    void svd22(const float a[4], float u[4], float s[2], float v[4]);
    
    /**
    * @brief 使用Umeyama算法计算仿射变换矩阵
    * @param src  原图像点位置
    * @param dst  目标图像（112*112）点位置。
    * @return None
    */
    void image_umeyama_112(float* src, float* dst);

    /**
    * @brief 获取affine变换矩阵
    * @param sparse_points  原图像人脸五官点位置
    * @return None
    */
    void get_affine_matrix(float* sparse_points);

    /**
    * @brief 使用L2范数对数据进行归一化
    * @param src  原始数据
    * @param dst  L2归一化后的数据
    * @param len  原始数据长度
    * @return None
    */
    void l2_normalize(float* src, float* dst, int len);

    /**
    * @brief 计算两特征的余弦距离
    * @param feature_0    第一个特征
    * @param feature_1    第二个特征
    * @param feature_len  特征长度
    * @return 余弦距离
    */
    float cal_cosine_distance(float* feature_0, float* feature_1, int feature_len);
    
    /**
    * @brief 获取目录下的文件个数
    * @param path     目标目录
    * @return 文件个数
    */
    int get_dir_files(const char *path);

    std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
    runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
    
    uintptr_t vaddr_;                            // isp的虚拟地址
    FrameCHWSize isp_shape_;                     // isp对应的地址大小
    float matrix_dst_[10];                       // 人脸affine的变换矩阵
    float obj_thresh_;                            // 人脸识别阈值
    int max_register_face_;                       // 数据库中最大存储人脸个数
    int feature_num_;                             // 人脸识别提取特征长度
public:
    float *feature_database_;                     // 人脸数据库数据
    vector<string> names_;                        // 人脸数据库名字
    int valid_register_face_;                     // 数据库中实际人脸个数
};
#endif