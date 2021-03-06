/*
                                +----------------------------------+
                                |                                  |
                                | ***  BA optimizer interface  *** |
                                |                                  |
                                |  Copyright (c) -tHE SWINe- 2015  |
                                |                                  |
                                |          BAOptimizer.h           |
                                |                                  |
                                +----------------------------------+
*/

#pragma once
#ifndef __BA_OPTIMIZER_INCLUDED
#define __BA_OPTIMIZER_INCLUDED

/**
 *	@file include/ba_interface_example/BAOptimizer.h
 *	@brief SLAM++ bundle adjustment optimizer interface
 *	@date 2014-03-20
 *	@author -tHE SWINe-
 */

#ifdef __cplusplus // this is required if you want to use SLAM++ from pure "C"

/**
 *	@def BA_OPTIMIZER_WANT_DEEP_VERTEX_ACCESS
 *	@brief if defined, it is possible to access optimizer vertex objects;
 *		otherwise only vertex state vectors can be accessed
 *	@note If not defined, the optimizer does not need to be included,
 *		resulting in much faster compilation times of files including this.
 */
//#define BA_OPTIMIZER_WANT_DEEP_VERTEX_ACCESS

#ifdef BA_OPTIMIZER_WANT_DEEP_VERTEX_ACCESS
#include "slam/LinearSolver_UberBlock.h"
#include "slam/ConfigSolvers.h" // nonlinear graph solvers
#include "slam/BA_Types.h"
#else // BA_OPTIMIZER_WANT_DEEP_VERTEX_ACCESS
#include "slam/BASolverBase.h" // C3DJacobians, CBAJacobians, generally useful functions for BA and SE(3), does not need to be included
#endif // BA_OPTIMIZER_WANT_DEEP_VERTEX_ACCESS

// replace so3 to quaternion
//#define ENABLE_QUATERNION

// enable good graph in stereo BA; not fully functioning
//#define ENABLE_STEREO_SLAMPP


/**
 *	@brief bundle adjustment optimizer
 *	@note The purpose is not to have any interaction with CSystemType or CNonlinearSolverType
 *		in the header, in order to move the slow building of SLAM++ to BAOptimizer.cpp, which
 *		is rarely changed throughout the development process.
 */
class CBAOptimizer {
public:
#ifdef BA_OPTIMIZER_WANT_DEEP_VERTEX_ACCESS
    typedef MakeTypelist_Safe((CVertexCam, CVertexXYZ)) TVertexTypelist;
    typedef MakeTypelist_Safe((CEdgeP2C3D)) TEdgeTypelist;
    typedef CFlatSystem<CSEBaseVertex, TVertexTypelist, CEdgeP2C3D, TEdgeTypelist> CSystemType;
#endif // BA_OPTIMIZER_WANT_DEEP_VERTEX_ACCESS

    class CBAOptimizerCore; // forward declaration

protected:
    CBAOptimizerCore *m_p_optimizer; // PIMPL

public:
    CBAOptimizer(bool b_verbose = false, bool b_use_schur = true); // throw(srd::bad_alloc) // todo - add more optimization settings as required
    ~CBAOptimizer();

    size_t n_Vertex_Num() const;

#ifdef SBA_OPTIMIZER_WANT_DEEP_VERTEX_ACCESS
    CSystemType::_TyConstVertexRef r_Vertex(size_t n_index) const; // returns const reference to a full vertex
    CSystemType::_TyVertexRef r_Vertex(size_t n_index); // returns reference to a full vertex
#endif // SBA_OPTIMIZER_WANT_DEEP_VERTEX_ACCESS

    Eigen::Map<const Eigen::VectorXd> r_Vertex_State(size_t n_index) const; // returns const map of vertex state only
    Eigen::Map<Eigen::VectorXd> r_Vertex_State(size_t n_index); // returns map of vertex state only

    void Delay_Optimization();
    void Enable_Optimization(); // throw(srd::bad_alloc, std::runtime_error)

    void Optimize(size_t n_max_iteration_num = 5, double f_min_dx_norm = .01); // throw(srd::bad_alloc, std::runtime_error)

    void Add_XYZVertex(size_t n_vertex_id, const Eigen::Vector3d &v_position); // throw(srd::bad_alloc)
    void Add_XYZVertex_Fixed(size_t n_vertex_id, const Eigen::Vector3d &v_position); // throw(srd::bad_alloc)

#ifdef ENABLE_STEREO_SLAMPP
    void Add_SCamVertex(size_t n_vertex_id, const Eigen::Matrix<double, 12, 1> &v_cam_state); // throw(srd::bad_alloc)
    void Add_SCamVertex_Fixed(size_t n_vertex_id, const Eigen::Matrix<double, 12, 1> &v_cam_state); // throw(srd::bad_alloc)
    void Add_P2SC3DEdge(size_t n_xyz_vertex_id, size_t n_cam_vertex_id,
                        const Eigen::Vector3d &v_observation, const Eigen::Matrix3d &t_information); // throw(srd::bad_alloc)
#elif defined ENABLE_QUATERNION
    void Add_CamVertex(size_t n_vertex_id, const Eigen::Matrix<double, 12, 1> &v_cam_state); // throw(srd::bad_alloc)
    void Add_CamVertex_Fixed(size_t n_vertex_id, const Eigen::Matrix<double, 12, 1> &v_cam_state); // throw(srd::bad_alloc)
    void Add_P2C3DEdge(size_t n_xyz_vertex_id, size_t n_cam_vertex_id,
                       const Eigen::Vector2d &v_observation, const Eigen::Matrix2d &t_information); // throw(srd::bad_alloc)
#else
    void Add_CamVertex(size_t n_vertex_id, const Eigen::Matrix<double, 11, 1> &v_cam_state); // throw(srd::bad_alloc)
    void Add_CamVertex_Fixed(size_t n_vertex_id, const Eigen::Matrix<double, 11, 1> &v_cam_state); // throw(srd::bad_alloc)
    void Add_P2C3DEdge(size_t n_xyz_vertex_id, size_t n_cam_vertex_id,
                       const Eigen::Vector2d &v_observation, const Eigen::Matrix2d &t_information); // throw(srd::bad_alloc)
#endif

    //
    double Find_Subgraph(/*const std::vector<size_t> & poses_vtx_, */
                         const size_t & card_, const size_t & greedy_method_, std::vector<size_t> & reserv_vtx_);
    //

    void Show_Stats() const;

    void resetTime();

    //    printf("\t     pre proc.: %f\n", m_f_pre_time);
    //    printf("\t  schur compl.: %f\n", m_f_schur_time);
    //    printf("\t  random query: %f\n", m_f_query_time);
    //    printf("\t       slicing: %f\n", m_f_slice_time);
    //    printf("\t      cholesky: %f\n", m_f_chol_time);
    //    printf("\t    post proc.: %f\n", m_f_post_time);
    void Dump_TimeLog(double & m_f_lambda_time, double & m_f_pre_time, double & m_f_schur_time, double & m_f_query_time,
                      double & m_f_slice_time, double & m_f_chol_time, double & m_f_post_time);

    bool Dump_State(const char *p_s_filename) const;

    bool Dump_Graph(const char *p_s_filename) const;

    // other types may be supported, as needed
    void Plot3D(const char *p_s_filename);

private:
    CBAOptimizer(const CBAOptimizer &r_other); // no copy
    CBAOptimizer &operator =(const CBAOptimizer &r_other); // no copy
};

#endif // __cplusplus

#if defined(ENABLE_BA_OPTIMIZER_C_INTERFACE) || !defined(__cplusplus)

#ifdef __cplusplus
extern "C" { // we might want to use the same interface from C++ (ENABLE_BA_OPTIMIZER_C_INTERFACE must be defined in the file which includes this, not necessarily for all the files)
#endif // __cplusplus

// this is a "C" interface for the same optimizer - now you can call SLAM++ from pure "C"
// in case your compiler doesnt know size_t, typedef unsigned long size_t could work

typedef struct _optimizer_t *optimizer_t; /**< @brief optimizer handle type */

optimizer_t New_Optimizer(int b_verbose, int b_use_schur);
void Free_Optimizer(optimizer_t h_optimizer);
size_t n_Vertex_Num(optimizer_t h_optimizer);
double *p_Vertex_State(optimizer_t h_optimizer, size_t n_vertex);
int n_Vertex_Dimension(optimizer_t h_optimizer, size_t n_vertex);
void Delay_Optimization(optimizer_t h_optimizer);
void Enable_Optimization(optimizer_t h_optimizer);
void Optimize(optimizer_t h_optimizer, size_t n_max_iteration_num, double f_min_dx_norm);
void Add_XYZVertex(optimizer_t h_optimizer, size_t n_vertex_id, const double p_position[3]);
#ifdef ENABLE_STEREO_SLAMPP
void Add_CamSVertex(optimizer_t h_optimizer, size_t n_vertex_id, const double p_cam_state[12]);
void Add_P2SC3DEdge(optimizer_t h_optimizer, size_t n_xyz_vertex_id, size_t n_cam_vertex_id,
                    const double p_observation[3], const double p_information[6]);
#else
void Add_CamVertex(optimizer_t h_optimizer, size_t n_vertex_id, const double p_cam_state[11]);
void Add_P2C3DEdge(optimizer_t h_optimizer, size_t n_xyz_vertex_id, size_t n_cam_vertex_id,
                   const double p_observation[2], const double p_information[4]);
#endif
int Dump_State(optimizer_t h_optimizer, const char *p_s_filename);
int Dump_Graph(optimizer_t h_optimizer, const char *p_s_filename);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ENABLE_BA_OPTIMIZER_C_INTERFACE || !__cplusplus

#endif // !__BA_OPTIMIZER_INCLUDED
