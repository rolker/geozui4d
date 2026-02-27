#include <gz4d/dsp/DiscreetFourierTransform.h>
#include <gz4d/dsp/FastFourierTransform.h>
#include <vector>
#include <complex>
#include <iostream>

#define DFT_N 32

int main(int argc, char* argv[])
{
    typedef double intype;
    typedef std::vector<intype> invectype;
    
    typedef std::complex<intype> outtype;
    typedef std::vector<outtype> outvectype;
    
    typedef std::back_insert_iterator<outvectype> outiitype;
    
    invectype timedomain;
    while(timedomain.size() < DFT_N)
    {
        intype v = timedomain.size()*2.0*M_PI/double(DFT_N);
        timedomain.push_back(cos(2.0*v)+1.2*cos(9.4*v));
    }
    
    outvectype frequencydomain;
    outvectype ffrequencydomain;
    
    outiitype ii(frequencydomain);
    outiitype fii(ffrequencydomain);
    
    gz4d::DiscreetFourierTransform<double>::transform(timedomain.begin(),ii,DFT_N);
    gz4d::FastFourierTransform<double,DFT_N>::transform(timedomain.begin(),fii);

    std::cout.setf(std::ios::fixed);
    for(int i = 0; i < DFT_N; ++i)
        std::cout << i/double(DFT_N) << "," << timedomain[i] << "," <<  abs(frequencydomain[i]) << "," << abs(ffrequencydomain[i]) << std::endl;
    
    std::cerr << frequencydomain.size() << "\t" << ffrequencydomain.size() << std::endl;

    //std::cerr << "Expi<1,2>: denom: " << gz4d::Expi< 1, 2>::denom << " xnum: " << gz4d::Expi< 1, 2>::xnum  << " xdenom: " << gz4d::Expi< 1, 2>::xdenom << std::endl; 
    
    return 0;
}
