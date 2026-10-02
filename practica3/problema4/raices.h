#ifndef RAICES_H
#define RAICES_H


class Raices {
  private:
    double a, b, c;
  
  public:
    Raices(double a, double b, double c);

    void setA(double a);
    void setB(double b);
    void setC(double c);
    double getA() const;
    double getB() const;
    double getC() const;

    double Discriminante();  
    bool DeterminarA();
    
    double Raiz1();
    double Raiz2();

};

#endif
