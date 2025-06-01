#pragma once

#if 1
#define _OBV_ISPC_
#else
#endif

#include "Indicator.h"

// it will be extreme inefficient to do this in ispc since current value is dependent on previous value
// which means calculations cannot be parallalized
class OBV : public Indicator
{
public:
	OBV() : Indicator() {}
	OBV(std::weak_ptr<Security> security) : Indicator(0, security) {};

	OBV(const OBV& other) = default;
	OBV(OBV&& other) noexcept = default;
	OBV& operator=(const OBV& other) = default;
	OBV& operator=(OBV&& other) noexcept = default;

	bool operator==(const OBV& other) const;

	virtual std::vector<std::vector<double>> Calculate() override;
	void CalculateRaw(const double* closes, const uint64_t* volumes, double* output, uint64_t data_size);

	virtual std::string GetName() const override { return std::format("On Balance Volume"); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_OBV; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<OBV>(*this);
	}

	virtual bool IsSingleInstanceType() const { return true; }
};

