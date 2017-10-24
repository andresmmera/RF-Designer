#ifndef LOWPASSPROTOTYPECOEFFS_H
#define LOWPASSPROTOTYPECOEFFS_H
#include <QtSql>
#include <deque>
#include "Filtering/Network.h"

class LowpassPrototypeCoeffs
{
public:
    LowpassPrototypeCoeffs(FilterSpecifications Specificica);
    std::deque<double> getCoefficients();
    void setDatabase(QSqlDatabase);

private:
    QSqlDatabase Zverev_db;
    FilterSpecifications Specification;

    std::deque<double> calcButterworth_gi();
    std::deque<double> calcChebyshev_gi();
};

#endif // LOWPASSPROTOTYPECOEFFS_H
