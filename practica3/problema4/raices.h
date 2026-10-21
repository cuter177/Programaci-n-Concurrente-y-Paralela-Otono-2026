#ifndef RAICES_H
#define RAICES_H

class Raices {
  private:
    double a, b, c;
    double b2, ac4, D, r, x1, x2;
    bool hay_raices;

  public:
    Raices(double a, double b, double c);

    void calcular();

    bool hayRaices() const;
    double getD() const;
    double getX1() const;
    double getX2() const;
};

#endif
