#ifndef _HANDLINGDATAMGR_H_
#define _HANDLINGDATAMGR_H_

class cHandlingDataMgr
{
	// NOT reversed data, just read from file
	struct Transmission {
		char nDriveType;
		char nEngineType;
		char nNumberOfGears;
		uchar Flags;
		float fEngineAcceleration;
		float fMaxVelocity;
	};
	struct Data {
		int ident;
		float fMass;
		rw::V3d Dimensions;
		rw::V3d CenterOfMass;
		int nPercentSubmerged;
		float fTractionMultiplier;
		float fTractionLoss;
		float fTractionBias;
		Transmission trans;

		float fBrakeDeceleration;
		float fBrakeBias;
		bool bABS;
		float fSteeringLock;
		float fSuspensionForceLevel;
		float fSuspensionDampingLevel;
		float fSeatOffsetDistance;
		float fCollisionDamageMultiplier;
		int nMonetaryValue;

		float fSuspensionUpperLimit;
		float fSuspensionLowerLimit;
		float fSuspensionBias;
		uchar Flags;
		uchar FrontLights;
		uchar RearLights;
	};
	static Data data[NUMHANDLINGS];
public:
	static void Initialise(void);
	static void LoadHandlingData(void);
	static int  GetHandlingData(const char *ident);
};

#endif
