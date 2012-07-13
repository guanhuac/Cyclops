/*
 * CyclicCoordinateDescent.h
 *
 *  Created on: May-June, 2010
 *      Author: msuchard
 */

#ifndef CYCLICCOORDINATEDESCENT_H_
#define CYCLICCOORDINATEDESCENT_H_

#include "CompressedDataMatrix.h"
#include "InputReader.h"
#include "ModelSpecifics.h"

//using namespace std;
using std::cout;
using std::cerr;
using std::endl;
using std::ostream;
using std::ofstream;

//#define DEBUG

#define TEST_SPARSE // New sparse updates are great
//#define TEST_ROW_INDEX
#define BETTER_LOOPS
#define MERGE_TRANSFORMATION
#define NEW_NUMERATOR
#define SPARSE_PRODUCT

#define USE_ITER


//#define NO_FUSE


#ifdef DOUBLE_PRECISION
	typedef double real;
#else
	typedef float real;
#endif 

enum PriorType {
	LAPLACE = 0,
	NORMAL  = 1
};

enum ConvergenceType {
	LANGE = 0,
	ZHANG_OLES = 1
};

class CyclicCoordinateDescent {
	
public:
	
	CyclicCoordinateDescent(void);
	
	CyclicCoordinateDescent(			
			const char* fileNameX,
			const char* fileNameEta,
			const char* fileNameOffs,
			const char* fileNameNEvents,
			const char* fileNamePid			
		);
	
	CyclicCoordinateDescent(
			InputReader* reader,
			AbstractModelSpecifics& specifics
		);

	CyclicCoordinateDescent(
			int inN,
			CompressedDataMatrix* inX,
			int* inEta, 
			int* inOffs, 
			int* inNEvents,
			int* inPid
		);
	
	void logResults(const char* fileName);

	virtual ~CyclicCoordinateDescent();
	
	double getLogLikelihood(void);

	double getPredictiveLogLikelihood(real* weights);

	double getLogPrior(void);
	
	virtual double getObjectiveFunction(void);

	real getBeta(int i);

	int getBetaSize(void);
		
	void update(int maxIterations, int convergenceType, double epsilon);

	virtual void resetBeta(void);

	// Setters
	void setHyperprior(double value);

	void setPriorType(int priorType);

	void setWeights(real* weights);

	void setLogisticRegression(bool idoLR);

//	template <typename T>
	void setBeta(const std::vector<double>& beta);

//	void double getHessianComponent(int i, int j);

	// Getters
	string getPriorInfo();

	string getConditionId() const {
		return conditionId;
	}

	int getUpdateCount() const {
		return updateCount;
	}

	int getLikelihoodCount() const {
		return likelihoodCount;
	}
		
protected:
	
	AbstractModelSpecifics& modelSpecifics;
//private:
	
	void init(void);
	
	void resetBounds(void);

	void computeXBeta(void);

	void saveXBeta(void);

	void computeXjEta(void);

	void computeSufficientStatistics(void);

	void updateSufficientStatistics(double delta, int index);

	void computeNumeratorForGradient(int index);

	virtual void computeNEvents(void);

	virtual void updateXBeta(double delta, int index);

	virtual void computeRemainingStatistics(bool skip, int index);
	
	virtual void computeRatiosForGradientAndHessian(int index);

	virtual void computeGradientAndHessian(
			int index,
			double *gradient,
			double *hessian);

	template <class IteratorType>
	void axpy(real* y, const real alpha, const int index);

	virtual void getDenominators(void);

	double computeLogLikelihood(void);

	double ccdUpdateBeta(int index);
	
	double applyBounds(
			double inDelta,
			int index);
	
	double computeConvergenceCriterion(double newObjFxn, double oldObjFxn);
	
	virtual double computeZhangOlesConvergenceCriterion(void);

	template <class T>
	void fillVector(T* vector, const int length, const T& value) {
		for (int i = 0; i < length; i++) {
			vector[i] = value;
		}
	}

	template <class T>
	void zeroVector(T* vector, const int length) {
		for (int i = 0; i < length; i++) {
			vector[i] = 0;
		}
	}

	int getAlignedLength(int N);
		
	void testDimension(int givenValue, int trueValue, const char *parameterName);
	
	template <class T>
	void printVector(T* vector, const int length, ostream &os);
	
	double oneNorm(real* vector, const int length);
	
	double twoNormSquared(real * vector, const int length); 
	
	int sign(double x); 
	
	template <class T> 
	T* readVector(const char *fileName, int *length); 
			
	// Local variables
	
	//InputReader* hReader;
	
	ofstream outLog;
	bool hasLog;

	CompressedDataMatrix* hXI; // K-by-J-indicator matrix

	int* hOffs;  // K-vector
	int* hEta; // K-vector
	int* hNEvents; // K-vector
	int* hPid; // N-vector
	int** hXColumnRowIndicators; // J-vector
 	
	real* hBeta;
	real* hXBeta;
	real* hXBetaSave;
	real* hDelta;

	int N; // Number of patients
	int K; // Number of exposure levels
	int J; // Number of drugs
	
	string conditionId;

	int priorType;
	double sigma2Beta;
	double lambda;

	real denomNullValue;

	bool sufficientStatisticsKnown;
	bool xBetaKnown;

	bool validWeights;
	bool useCrossValidation;
	bool doLogisticRegression;
	real* hWeights;

	// temporary variables
	real* expXBeta;
	real* offsExpXBeta;
	real* denomPid;
	real* numerPid;
	real* numerPid2;
	real* xOffsExpXBeta;
	real* hXjEta;

	int updateCount;
	int likelihoodCount;

#ifdef SPARSE_PRODUCT
	std::vector<std::vector<int>* > sparseIndices;
#endif
	
#ifdef NO_FUSE
	real* wPid;
#endif
};

double convertVarianceToHyperparameter(double variance);

#endif /* CYCLICCOORDINATEDESCENT_H_ */
