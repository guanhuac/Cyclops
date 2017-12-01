/*
 * Kernels.hpp
 *
 *  Created on: Apr 4, 2016
 *      Author: msuchard
 */

#ifndef KERNELS_HPP_
#define KERNELS_HPP_

#include <boost/compute/type_traits/type_name.hpp>

namespace bsccs {


namespace {

template<typename T, bool isNvidiaDevice>
struct ReduceBody1 {
    static std::string body() {
        std::stringstream k;
        // local reduction for non-NVIDIA device
        k <<
            "   for(int j = 1; j < TPB; j <<= 1) {        \n" <<
            "       barrier(CLK_LOCAL_MEM_FENCE);         \n" <<
            "       uint mask = (j << 1) - 1;             \n" <<
            "       if ((lid & mask) == 0) {              \n" <<
            "           scratch[lid] += scratch[lid + j]; \n" <<
            "       }                                     \n" <<
            "   }                                         \n";
        return k.str();
    }
};

template<typename T, bool isNvidiaDevice>
struct ReduceBody2 {
    static std::string body() {
        std::stringstream k;
        // local reduction for non-NVIDIA device
        k <<
            "   for(int j = 1; j < TPB; j <<= 1) {          \n" <<
            "       barrier(CLK_LOCAL_MEM_FENCE);           \n" <<
            "       uint mask = (j << 1) - 1;               \n" <<
            "       if ((lid & mask) == 0) {                \n" <<
            "           scratch[0][lid] += scratch[0][lid + j]; \n" <<
            "           scratch[1][lid] += scratch[1][lid + j]; \n" <<
            "       }                                       \n" <<
            "   }                                           \n";
        return k.str();
    }
};

template<typename T>
struct ReduceBody1<T,true>
{
    static std::string body() {
        std::stringstream k;
        k <<
            "   barrier(CLK_LOCAL_MEM_FENCE);\n" <<
            "   if (TPB >= 1024) { if (lid < 512) { sum += scratch[lid + 512]; scratch[lid] = sum; } barrier(CLK_LOCAL_MEM_FENCE); } \n" <<
            "   if (TPB >=  512) { if (lid < 256) { sum += scratch[lid + 256]; scratch[lid] = sum; } barrier(CLK_LOCAL_MEM_FENCE); } \n" <<
            "   if (TPB >=  256) { if (lid < 128) { sum += scratch[lid + 128]; scratch[lid] = sum; } barrier(CLK_LOCAL_MEM_FENCE); } \n" <<
            "   if (TPB >=  128) { if (lid <  64) { sum += scratch[lid +  64]; scratch[lid] = sum; } barrier(CLK_LOCAL_MEM_FENCE); } \n" <<
        // warp reduction
            "   if (lid < 32) { \n" <<
        // volatile this way we don't need any barrier
            "       volatile __local TMP_REAL *lmem = scratch;                 \n" <<
            "       if (TPB >= 64) { lmem[lid] = sum = sum + lmem[lid + 32]; } \n" <<
            "       if (TPB >= 32) { lmem[lid] = sum = sum + lmem[lid + 16]; } \n" <<
            "       if (TPB >= 16) { lmem[lid] = sum = sum + lmem[lid +  8]; } \n" <<
            "       if (TPB >=  8) { lmem[lid] = sum = sum + lmem[lid +  4]; } \n" <<
            "       if (TPB >=  4) { lmem[lid] = sum = sum + lmem[lid +  2]; } \n" <<
            "       if (TPB >=  2) { lmem[lid] = sum = sum + lmem[lid +  1]; } \n" <<
            "   }                                                            \n";
        return k.str();
    }
};

template<typename T>
struct ReduceBody2<T,true>
{
    static std::string body() {
        std::stringstream k;
        k <<
            "   barrier(CLK_LOCAL_MEM_FENCE); \n" <<
            "   if (TPB >= 1024) { if (lid < 512) { sum0 += scratch[0][lid + 512]; scratch[0][lid] = sum0; sum1 += scratch[1][lid + 512]; scratch[1][lid] = sum1; } barrier(CLK_LOCAL_MEM_FENCE); } \n" <<
            "   if (TPB >=  512) { if (lid < 256) { sum0 += scratch[0][lid + 256]; scratch[0][lid] = sum0; sum1 += scratch[1][lid + 256]; scratch[1][lid] = sum1; } barrier(CLK_LOCAL_MEM_FENCE); } \n" <<
            "   if (TPB >=  256) { if (lid < 128) { sum0 += scratch[0][lid + 128]; scratch[0][lid] = sum0; sum1 += scratch[1][lid + 128]; scratch[1][lid] = sum1; } barrier(CLK_LOCAL_MEM_FENCE); } \n" <<
            "   if (TPB >=  128) { if (lid <  64) { sum0 += scratch[0][lid +  64]; scratch[0][lid] = sum0; sum1 += scratch[1][lid +  64]; scratch[1][lid] = sum1; } barrier(CLK_LOCAL_MEM_FENCE); } \n" <<
        // warp reduction
            "   if (lid < 32) { \n" <<
        // volatile this way we don't need any barrier
            "       volatile __local TMP_REAL *lmem = scratch; \n" <<
            //"       volatile __local TMP_REAL *lmem1 = scratch1; \n" <<
            "       if (TPB >= 64) { lmem[0][lid] = sum0 = sum0 + lmem[0][lid+32]; lmem[1][lid] = sum1 = sum1 + lmem[1][lid+32]; } \n" <<
            "       if (TPB >= 32) { lmem[0][lid] = sum0 = sum0 + lmem[0][lid+16]; lmem[1][lid] = sum1 = sum1 + lmem[1][lid+16]; } \n" <<
            "       if (TPB >= 16) { lmem[0][lid] = sum0 = sum0 + lmem[0][lid+ 8]; lmem[1][lid] = sum1 = sum1 + lmem[1][lid+ 8]; } \n" <<
            "       if (TPB >=  8) { lmem[0][lid] = sum0 = sum0 + lmem[0][lid+ 4]; lmem[1][lid] = sum1 = sum1 + lmem[1][lid+ 4]; } \n" <<
            "       if (TPB >=  4) { lmem[0][lid] = sum0 = sum0 + lmem[0][lid+ 2]; lmem[1][lid] = sum1 = sum1 + lmem[1][lid+ 2]; } \n" <<
            "       if (TPB >=  2) { lmem[0][lid] = sum0 = sum0 + lmem[0][lid+ 1]; lmem[1][lid] = sum1 = sum1 + lmem[1][lid+ 1]; } \n" <<
            "   }                                            \n";
        return k.str();
    }
};

template <class BaseModel> // if BaseModel derives from IndependentData
typename std::enable_if<std::is_base_of<IndependentData,BaseModel>::value, std::string>::type
static group(const std::string& id, const std::string& k) {
    return k;
};

template <class BaseModel> // if BaseModel does not derive from IndependentData
typename std::enable_if<!std::is_base_of<IndependentData,BaseModel>::value, std::string>::type
static group(const std::string& id, const std::string& k) {
    return id + "[" + k + "]";
};

static std::string timesX(const std::string& arg, const FormatType formatType) {
    return (formatType == INDICATOR || formatType == INTERCEPT) ?
        arg : arg + " * x";
}

static std::string weight(const std::string& arg, bool useWeights) {
    return useWeights ? "w * " + arg : arg;
}

}; // anonymous namespace

	template <class BaseModel, typename WeightType>
    SourceCode
    GpuModelSpecifics<BaseModel, WeightType>::writeCodeForGradientHessianKernel(FormatType formatType, bool useWeights, bool isNvidia) {

        std::string name = "computeGradHess" + getFormatTypeExtension(formatType) + (useWeights ? "W" : "N");

        std::stringstream code;
        code << "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n";

        code << "__kernel void " << name << "(            \n" <<
                "       const uint offX,                  \n" <<
                "       const uint offK,                  \n" <<
                "       const uint N,                     \n" <<
                "       __global const REAL* X,           \n" <<
                "       __global const int* K,            \n" <<
                "       __global const REAL* Y,           \n" <<
                "       __global const REAL* xBeta,       \n" <<
                "       __global const REAL* expXBeta,    \n" <<
                "       __global const REAL* denominator, \n" <<
#ifdef USE_VECTOR
                "       __global TMP_REAL* buffer,     \n" <<
#else
                "       __global REAL* buffer,            \n" <<
#endif // USE_VECTOR
                "       __global const int* id,           \n" <<  // TODO Make id optional
                "       __global const REAL* weight) {    \n";    // TODO Make weight optional

        // Initialization
        code << "   const uint lid = get_local_id(0); \n" <<
                "   const uint loopSize = get_global_size(0); \n" <<
                "   uint task = get_global_id(0);  \n" <<
                    // Local and thread storage
#ifdef USE_VECTOR
                "   __local TMP_REAL scratch[TPB]; \n" <<
                "   TMP_REAL sum = 0.0;            \n" <<
#else
                "   __local REAL scratch[2][TPB + 1];  \n" <<
               // "   __local REAL scratch1[TPB];  \n" <<
                "   REAL sum0 = 0.0; \n" <<
                "   REAL sum1 = 0.0; \n" <<
#endif // USE_VECTOR
                    //
                "   while (task < N) { \n";

        // Fused transformation-reduction

        if (formatType == INDICATOR || formatType == SPARSE) {
            code << "       const uint k = K[offK + task];         \n";
        } else { // DENSE, INTERCEPT
            code << "       const uint k = task;            \n";
        }

        if (formatType == SPARSE || formatType == DENSE) {
            code << "       const REAL x = X[offX + task]; \n";
        } else { // INDICATOR, INTERCEPT
            // Do nothing
        }

        code << "       const REAL exb = expXBeta[k];     \n" <<
                "       const REAL numer = " << timesX("exb", formatType) << ";\n" <<
                "       const REAL denom = 1.0 + exb;      \n" <<
        //denominator[k]; \n" <<
                "       const REAL g = numer / denom;      \n";

        if (useWeights) {
            code << "       const REAL w = weight[k];\n";
        }

        code << "       const REAL gradient = " << weight("g", useWeights) << ";\n";
        if (formatType == INDICATOR || formatType == INTERCEPT) {
            code << "       const REAL hessian  = " << weight("g * ((REAL)1.0 - g)", useWeights) << ";\n";
        } else {
            code << "       const REAL nume2 = " << timesX("numer", formatType) << ";\n" <<
                    "       const REAL hessian  = " << weight("(nume2 / denom - g * g)", useWeights) << ";\n";
        }

#ifdef USE_VECTOR
        code << "       sum += (TMP_REAL)(gradient, hessian); \n";
#else
        code << "       sum0 += gradient; \n" <<
                "       sum1 += hessian;  \n";
#endif // USE_VECTOR

        // Bookkeeping
        code << "       task += loopSize; \n" <<
                "   } \n" <<
                    // Thread -> local
#ifdef USE_VECTOR
                "   scratch[lid] = sum; \n";
#else
                "   scratch[0][lid] = sum0; \n" <<
                "   scratch[1][lid] = sum1; \n";
#endif // USE_VECTOR

#ifdef USE_VECTOR
        // code << (isNvidia ? ReduceBody1<real,true>::body() : ReduceBody1<real,false>::body());
        code << ReduceBody1<real,false>::body();
#else
        code << (isNvidia ? ReduceBody2<real,true>::body() : ReduceBody2<real,false>::body());
#endif

        code << "   if (lid == 0) { \n" <<
#ifdef USE_VECTOR
                "       buffer[get_group_id(0)] = scratch[0]; \n" <<
#else
                "       buffer[get_group_id(0)] = scratch[0][0]; \n" <<
                "       buffer[get_group_id(0) + get_num_groups(0)] = scratch[1][0]; \n" <<
#endif // USE_VECTOR
                "   } \n";

        code << "}  \n"; // End of kernel

        return SourceCode(code.str(), name);
    }


	template <class BaseModel, typename WeightType>
    SourceCode
    GpuModelSpecifics<BaseModel, WeightType>::writeCodeForUpdateXBetaKernel(FormatType formatType) {

        std::string name = "updateXBeta" + getFormatTypeExtension(formatType);

        std::stringstream code;
        code << "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n";

        code << "__kernel void " << name << "(     \n" <<
                "       const uint offX,           \n" <<
                "       const uint offK,           \n" <<
                "       const uint N,              \n" <<
                "       const REAL delta,          \n" <<
                "       __global const REAL* X,    \n" <<
                "       __global const int* K,     \n" <<
                "       __global const REAL* Y,    \n" <<
                "       __global REAL* xBeta,      \n" <<
                "       __global REAL* expXBeta,   \n" <<
                "       __global REAL* denominator,\n" <<
                "       __global const int* id) {   \n" <<
                "   const uint task = get_global_id(0); \n";

        if (formatType == INDICATOR || formatType == SPARSE) {
            code << "   const uint k = K[offK + task];         \n";
        } else { // DENSE, INTERCEPT
            code << "   const uint k = task;            \n";
        }

        if (formatType == SPARSE || formatType == DENSE) {
            code << "   const REAL inc = delta * X[offX + task]; \n";
        } else { // INDICATOR, INTERCEPT
            code << "   const REAL inc = delta;           \n";
        }

        code << "   if (task < N) {      				\n";
        code << "       REAL xb = xBeta[k] + inc; 		\n" <<
                "       xBeta[k] = xb;                  \n";

        if (BaseModel::likelihoodHasDenominator) {
            // TODO: The following is not YET thread-safe for multi-row observations
            // code << "       const REAL oldEntry = expXBeta[k];                 \n" <<
            //         "       const REAL newEntry = expXBeta[k] = exp(xBeta[k]); \n" <<
            //         "       denominator[" << group<BaseModel>("id","k") << "] += (newEntry - oldEntry); \n";


            code << "       const REAL exb = exp(xb); \n" <<
                    "       expXBeta[k] = exb;        \n";
        	//code << "expXBeta[k] = exp(xb); \n";
        	//code << "expXBeta[k] = exp(1); \n";

            // LOGISTIC MODEL ONLY
            //                     const real t = BaseModel::getOffsExpXBeta(offs, xBeta[k], y[k], k);
            //                     expXBeta[k] = t;
            //                     denominator[k] = static_cast<real>(1.0) + t;
            //             code << "    const REAL t = 0.0;               \n" <<
            //                     "   expXBeta[k] = exp(xBeta[k]);      \n" <<
            //                     "   denominator[k] = REAL(1.0) + tmp; \n";
        }

        code << "   } \n";
        code << "}    \n";

        return SourceCode(code.str(), name);
    }

	template <class BaseModel, typename WeightType>
    SourceCode
    GpuModelSpecifics<BaseModel, WeightType>::writeCodeForComputeRemainingStatisticsKernel(FormatType formatType) {

        std::string name = "computeRemainingStatistics" + getFormatTypeExtension(formatType);

        std::stringstream code;
        code << "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n";

        code << "__kernel void " << name << "(     \n" <<
                "       const uint N,              \n" <<
				"		__global REAL* xBeta,	   \n" <<
                "       __global REAL* expXBeta,   \n" <<
                "       __global REAL* denominator,\n" <<
                "       __global const int* id) {   \n" <<
                "   const uint task = get_global_id(0); \n";
        //code << "   const uint lid = get_local_id(0); \n" <<
        //        "   const uint loopSize = get_global_size(0); \n";
        // Local and thread storage
        code << "   if (task < N) {      				\n";
        if (BaseModel::likelihoodHasDenominator) {
        	code << " 		REAL exb = exp(xBeta[task]);		\n" <<
        			"		expXBeta[task] = exb;		\n";
            code << "       denominator[task] = 1.0 + exb; \n";// <<
        	//code << "expXBeta[k] = exp(xb); \n";
        	//code << "expXBeta[k] = exp(1); \n";

            // LOGISTIC MODEL ONLY
            //                     const real t = BaseModel::getOffsExpXBeta(offs, xBeta[k], y[k], k);
            //                     expXBeta[k] = t;
            //                     denominator[k] = static_cast<real>(1.0) + t;
            //             code << "    const REAL t = 0.0;               \n" <<
            //                     "   expXBeta[k] = exp(xBeta[k]);      \n" <<
            //                     "   denominator[k] = REAL(1.0) + tmp; \n";
        }

        code << "   } \n";
        code << "}    \n";

        return SourceCode(code.str(), name);
    }

	template <class BaseModel, typename WeightType>
    SourceCode
    GpuModelSpecifics<BaseModel, WeightType>::writeCodeForComputeXBetaKernel(FormatType formatType) {

        std::string name = "computeXBeta" + getFormatTypeExtension(formatType);


        std::stringstream code;
        code << "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n";

        code << "__kernel void " << name << "(     \n" <<
                "       const uint offX,           \n" <<
                "       const uint offK,           \n" <<
                "       const uint N,              \n" <<
                "       const REAL delta,          \n" <<
                "       __global const REAL* X,    \n" <<
                "       __global const int* K,     \n" <<
                "       __global const REAL* Y,    \n" <<
                "       __global REAL* xBeta,      \n" <<
                "       __global REAL* expXBeta,   \n" <<
                "       __global REAL* denominator,\n" <<
                "       __global const int* id) {   \n" <<
                "   const uint task = get_global_id(0); \n";

        if (formatType == INDICATOR || formatType == SPARSE) {
            code << "   const uint k = K[offK + task];         \n";
        } else { // DENSE, INTERCEPT
            code << "   const uint k = task;            \n";
        }

        if (formatType == SPARSE || formatType == DENSE) {
            code << "   const REAL inc = delta * X[offX + task]; \n";
        } else { // INDICATOR, INTERCEPT
            code << "   const REAL inc = delta;           \n";
        }

        code << "   if (task < N) {      				\n";
        code << "       REAL xb = inc; 		\n" <<
                "       xBeta[k] = xb;                  \n";

        if (BaseModel::likelihoodHasDenominator) {
            // TODO: The following is not YET thread-safe for multi-row observations
            // code << "       const REAL oldEntry = expXBeta[k];                 \n" <<
            //         "       const REAL newEntry = expXBeta[k] = exp(xBeta[k]); \n" <<
            //         "       denominator[" << group<BaseModel>("id","k") << "] += (newEntry - oldEntry); \n";


            code << "       REAL exb = exp(xb); \n" <<
                    "       expXBeta[k] = exb;        \n";
        	//code << "expXBeta[k] = exp(xb); \n";
        	//code << "expXBeta[k] = exp(1); \n";

            // LOGISTIC MODEL ONLY
            //                     const real t = BaseModel::getOffsExpXBeta(offs, xBeta[k], y[k], k);
            //                     expXBeta[k] = t;
            //                     denominator[k] = static_cast<real>(1.0) + t;
            //             code << "    const REAL t = 0.0;               \n" <<
            //                     "   expXBeta[k] = exp(xBeta[k]);      \n" <<
            //                     "   denominator[k] = REAL(1.0) + tmp; \n";
        }

        code << "   } \n";
        code << "}    \n";

        return SourceCode(code.str(), name);
    }

	template <class BaseModel, typename WeightType>
    SourceCode
	GpuModelSpecifics<BaseModel, WeightType>::writeCodeForMMGradientHessianKernel(FormatType formatType, bool useWeights, bool isNvidia) {

        std::string name = "computeMMGradHess" + getFormatTypeExtension(formatType) + (useWeights ? "W" : "N");

        std::stringstream code;
        code << "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n";

        code << "__kernel void " << name << "(            \n" <<
                "       const uint offX,                  \n" <<
                "       const uint offK,                  \n" <<
                "       const uint N,                     \n" <<
                "       __global const REAL* X,           \n" <<
                "       __global const int* K,            \n" <<
                "       __global const REAL* Y,           \n" <<
                "       __global const REAL* xBeta,       \n" <<
                "       __global const REAL* expXBeta,    \n" <<
                "       __global const REAL* denominator, \n" <<
#ifdef USE_VECTOR
                "       __global TMP_REAL* buffer,     \n" <<
#else
                "       __global REAL* buffer,            \n" <<
#endif // USE_VECTOR
                "       __global const int* id,           \n" <<  // TODO Make id optional
                "       __global const REAL* weight,	  \n" <<
				"		__global const REAL* norm,		  \n" <<
				//"		__global const REAL* fixedBeta,   \n" <<
				"		const uint index) {    \n";    // TODO Make weight optional


        // Initialization
        code << "   const uint lid = get_local_id(0); \n" <<
                "   const uint loopSize = get_global_size(0); \n" <<
                "   uint task = get_global_id(0);  \n" <<
                    // Local and thread storage
#ifdef USE_VECTOR
                "   __local TMP_REAL scratch[TPB]; \n" <<
                "   TMP_REAL sum = 0.0;            \n" <<
#else
                "   __local REAL scratch[2][TPB + 1];  \n" <<
               // "   __local REAL scratch1[TPB];  \n" <<
                "   REAL sum0 = 0.0; \n" <<
                "   REAL sum1 = 0.0; \n" <<

#endif // USE_VECTOR
                "   while (task < N) { \n";

        // Fused transformation-reduction

        if (formatType == INDICATOR || formatType == SPARSE) {
            code << "       const uint k = K[offK + task];         \n";
        } else { // DENSE, INTERCEPT
            code << "       const uint k = task;            \n";
        }

        if (formatType == SPARSE || formatType == DENSE) {
            code << "       const REAL x = X[offX + task]; \n";
        } else { // INDICATOR, INTERCEPT
            // Do nothing
        }

        code << "       const REAL exb = expXBeta[k];     	\n" <<
        		"		const REAL xb = xBeta[k];			\n" <<
				"		const REAL norm0 = norm[k];				\n" <<
                "       const REAL numer = " << timesX("exb", formatType) << ";\n" <<
				//"		const REAL denom = denominator[k];	\n";
        		"		const REAL denom = 1 + exb;			\n";
				//"		const REAL factor = norm[k]/abs(x);				\n" <<
        //denominator[k]; \n" <<
                //"       const REAL g = numer / denom;      \n";

        if (useWeights) {
            code << "       const REAL w = weight[k];\n";
        }

        code << "       const REAL gradient = " << weight("numer / denom", useWeights) << ";\n";
        code << "		REAL hessian = 0.0;			\n";


        code << "if (x != 0.0) { \n";
        if (formatType == INDICATOR || formatType == INTERCEPT) {
            code << "       hessian  = " << weight("g  * norm0 / fabs(x) / denom / denom ", useWeights) << ";\n";
        } else {
            code << "       const REAL nume2 = " << timesX("numer", formatType) << ";\n" <<
            		"       hessian  = " << weight("nume2 * norm0 / fabs(x) / denom / denom", useWeights) << ";\n";
        }
        code << "} \n";

#ifdef USE_VECTOR
        code << "       sum += (TMP_REAL)(gradient, hessian); \n";
#else
        code << "       sum0 += gradient; \n" <<
                "       sum1 += hessian;  \n";
#endif // USE_VECTOR

        // Bookkeeping
        code << "       task += loopSize; \n" <<
                "   } \n" <<
                    // Thread -> local
#ifdef USE_VECTOR
                "   scratch[lid] = sum; \n";
#else
                "   scratch[0][lid] = sum0; \n" <<
                "   scratch[1][lid] = sum1; \n";
#endif // USE_VECTOR

#ifdef USE_VECTOR
        // code << (isNvidia ? ReduceBody1<real,true>::body() : ReduceBody1<real,false>::body());
        code << ReduceBody1<real,false>::body();
#else
        code << (isNvidia ? ReduceBody2<real,true>::body() : ReduceBody2<real,false>::body());
#endif

        code << "   if (lid == 0) { \n" <<
#ifdef USE_VECTOR
                "       buffer[get_group_id(0)] = scratch[0]; \n" <<
#else
                "       buffer[get_group_id(0) + 2*get_num_groups(0)*index] = scratch[0][0]; \n" <<
                "       buffer[get_group_id(0) + get_num_groups(0) + 2*get_num_groups(0)*index] = scratch[1][0]; \n" <<
#endif // USE_VECTOR
                "   } \n";


        code << "}  \n"; // End of kernel

        return SourceCode(code.str(), name);
	}

/*
	template <class BaseModel, typename WeightType>
    SourceCode
	GpuModelSpecifics<BaseModel, WeightType>::writeCodeForMMGradientHessianKernel(FormatType formatType, bool useWeights, bool isNvidia) {

        std::string name = "computeMMGradHess" + getFormatTypeExtension(formatType) + (useWeights ? "W" : "N");

        std::stringstream code;
        code << "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n";

        code << "__kernel void " << name << "(            \n" <<
                "       __global const uint* offXVec,                  \n" <<
                "       __global const uint* offKVec,                  \n" <<
                "       __global const uint* NVec,                     \n" <<
                "       __global const REAL* X,           \n" <<
                "       __global const int* K,            \n" <<
                "       __global const REAL* Y,           \n" <<
                "       __global const REAL* xBeta,       \n" <<
                "       __global const REAL* expXBeta,    \n" <<
                "       __global const REAL* denominator, \n" <<
#ifdef USE_VECTOR
                "       __global TMP_REAL* buffer,     \n" <<
#else
                "       __global REAL* buffer,            \n" <<
#endif // USE_VECTOR
                "       __global const int* id,           \n" <<  // TODO Make id optional
                "       __global const REAL* weight,	  \n" <<
				"		__global const REAL* norm,		  \n" <<
				"		__global const bool* fixBeta,   \n" <<
				"		const uint J) {    \n";    // TODO Make weight optional


        // Initialization
        code << "   const uint lid = get_local_id(0); \n" <<
                "   const uint loopSize = get_global_size(0); \n" <<
                //"   uint task = get_global_id(0);  \n" <<
                "   __local REAL scratch[2][TPB + 1];  \n" <<
				"	for (int index = 0; index < J; index++) { 	\n"
                "   uint task = get_global_id(0);  \n" <<
				"	barrier(CLK_GLOBAL_MEM_FENCE);				\n" <<
				//"	if (index == 1) {printf(\"%s\\n\", \"this is a test string\\n\");} \n" <<
				"	if (fixBeta[index]) continue;				\n" <<
				"	const uint offX = offXVec[index];			\n" <<
				"	const uint offK = offKVec[index];			\n" <<
				"	const uint N = NVec[index];					\n" <<
                    // Local and thread storage
#ifdef USE_VECTOR
                "   __local TMP_REAL scratch[TPB]; \n" <<
                "   TMP_REAL sum = 0.0;            \n" <<
#else
               // "   __local REAL scratch1[TPB];  \n" <<
                "   REAL sum0 = 0.0; \n" <<
                "   REAL sum1 = 0.0; \n" <<
				//"	if (lid == 0) printf(\"index: %d N: %d \", index, N); \n" <<

#endif // USE_VECTOR
                "   while (task < N) { \n";

        // Fused transformation-reduction

        if (formatType == INDICATOR || formatType == SPARSE) {
            code << "       const uint k = K[offK + task];         \n";
        } else { // DENSE, INTERCEPT
            code << "       const uint k = task;            \n";
        }

        if (formatType == SPARSE || formatType == DENSE) {
            code << "       const REAL x = X[offX + task]; \n";
        } else { // INDICATOR, INTERCEPT
            // Do nothing
        }

        code << "       const REAL exb = expXBeta[k];     	\n" <<
        		"		const REAL xb = xBeta[k];			\n" <<
				"		const REAL norm0 = norm[k];				\n" <<
                "       const REAL numer = " << timesX("exb", formatType) << ";\n" <<
				//"		const REAL denom = denominator[k];	\n";
        		"		const REAL denom = 1 + exb;			\n";
				//"		const REAL factor = norm[k]/abs(x);				\n" <<
        //denominator[k]; \n" <<
                //"       const REAL g = numer / denom;      \n";

        if (useWeights) {
            code << "       const REAL w = weight[k];\n";
        }

        code << "       const REAL gradient = " << weight("numer / denom", useWeights) << ";\n";
        code << "		REAL hessian = 0.0;			\n";


        code << "if (x != 0.0) { \n";
        if (formatType == INDICATOR || formatType == INTERCEPT) {
            code << "       hessian  = " << weight("g  * norm0 / fabs(x) / denom / denom ", useWeights) << ";\n";
        } else {
            code << "       const REAL nume2 = " << timesX("numer", formatType) << ";\n" <<
            		"       hessian  = " << weight("nume2 * norm0 / fabs(x) / denom / denom", useWeights) << ";\n";
        }
        code << "} \n";

#ifdef USE_VECTOR
        code << "       sum += (TMP_REAL)(gradient, hessian); \n";
#else
        code << "       sum0 += gradient; \n" <<
                "       sum1 += hessian;  \n";
#endif // USE_VECTOR

        // Bookkeeping
        code << "       task += loopSize; \n" <<
                "   } \n" <<
                    // Thread -> local
#ifdef USE_VECTOR
                "   scratch[lid] = sum; \n";
#else
                "   scratch[0][lid] = sum0; \n" <<
                "   scratch[1][lid] = sum1; \n";
#endif // USE_VECTOR

#ifdef USE_VECTOR
        // code << (isNvidia ? ReduceBody1<real,true>::body() : ReduceBody1<real,false>::body());
        code << ReduceBody1<real,false>::body();
#else
        code << (isNvidia ? ReduceBody2<real,true>::body() : ReduceBody2<real,false>::body());
#endif

        code << "   if (lid == 0) { \n" <<
				//"	printf(\"%f %f | \", scratch[0][0], scratch[1][0]); \n" <<
#ifdef USE_VECTOR
                "       buffer[get_group_id(0)] = scratch[0]; \n" <<
#else
                "       buffer[get_group_id(0) + 2*get_num_groups(0)*index] = scratch[0][0]; \n" <<
                "       buffer[get_group_id(0) + get_num_groups(0) + 2*get_num_groups(0)*index] = scratch[1][0]; \n" <<
#endif // USE_VECTOR
                "   } \n";
        code << "}\n";


        code << "}  \n"; // End of kernel

        return SourceCode(code.str(), name);
	}
	*/

	template <class BaseModel, typename WeightType>
    SourceCode
	GpuModelSpecifics<BaseModel, WeightType>::writeCodeForGetGradientObjective(FormatType formatType, bool useWeights, bool isNvidia) {
        std::string name = "getGradientObjective" + getFormatTypeExtension(formatType) + (useWeights ? "W" : "N");

        std::stringstream code;
        code << "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n";

        code << "__kernel void " << name << "(            \n" <<
                "       const uint N,                     \n" <<
                "       __global const REAL* Y,           \n" <<
                "       __global const REAL* xBeta,       \n" <<
                "       __global REAL* buffer,            \n" <<
                "       __global const REAL* weight) {    \n";    // TODO Make weight optional
        // Initialization
        code << "   const uint lid = get_local_id(0); \n" <<
                "   const uint loopSize = get_global_size(0); \n" <<
                "   uint task = get_global_id(0);  \n" <<
                    // Local and thread storage
                "   __local REAL scratch[TPB + 1];  \n" <<
                "   REAL sum = 0.0; \n" <<
                "   while (task < N) { \n";
        if (useWeights) {
        	code << "       const REAL w = weight[task];\n";
        }
        code << "	const REAL xb = xBeta[task];     \n" <<
        		"	const REAL y = Y[task];			 \n";
        code << " sum += " << weight("y * xb", useWeights) << ";\n";
        // Bookkeeping
        code << "       task += loopSize; \n" <<
                "   } \n" <<
                    // Thread -> local
                "   scratch[lid] = sum; \n";
        code << (isNvidia ? ReduceBody1<real,true>::body() : ReduceBody1<real,false>::body());

        code << "   if (lid == 0) { \n" <<
                "       buffer[get_group_id(0)] = scratch[0]; \n" <<
                "   } \n";
        code << "}  \n"; // End of kernel
        return SourceCode(code.str(), name);
	}

} // namespace bsccs

#endif /* KERNELS_HPP_ */




