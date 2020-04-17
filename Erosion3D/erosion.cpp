#include <ctime>
#include <random>
#include <iostream>
#include "erosion.h"
#include "PerlinNoise.h"

template <class T>
T max(T a, T b) {
    return (a > b ? a : b);
}

template <class T>
T min(T a, T b) {
    return (a < b ? a : b);
}

void Grid::Normalize()
{
    double min = arr[0];
    double max = arr[0];

    for (int i = 0; i < length; i++)
    {
        if (!isnan(arr[i]))
            if (arr[i] > max)
                max = arr[i];
        if (arr[i] < min)
            min = arr[i];
    }

    double mult = (max - min);
    for (int i = 0; i < length; i++)
    {
        double temp = arr[i];
        double temp1 = temp - min;
        double temp2 = temp1 / mult;

        if (isnan(temp2))
            arr[i] = 0;
        else
            arr[i] = temp2;
    }
}

void Grid::Brush(int centreX, int centreY, int radius, double val)
{
    double weightSum = 0;
    int addIndex = 0;
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            float sqrDst = x * x + y * y;
            if (sqrDst < radius * radius) {
                int coordX = centreX + x;
                int coordY = centreY + y;

                float weight = 1 - sqrt(sqrDst) / radius;
                weightSum += weight;
                addIndex++;

                if (coordX < rows && coordX >= 0 && coordY < columns && coordY >= 0)
                    AddVal(coordX, coordY, -weight * val);
            }
        }
    }
}

void Grid::Blur(double f1)
{
    std::vector<double> temp = arr;
    double f2 = (5.0 - f1) / 4.0;

    for (int i = 1; i < rows - 1; i++)
    {
        for (int j = 1; j < columns - 1; j++)
        {
            temp[j * rows + i] = (GetVal(i - 1, j) * f2 + f1 * GetVal(i, j) + f2 * GetVal(i + 1, j) + GetVal(i, j+1) * f2 + GetVal(i, j-1) * f2) / 5.0;
        }
    }

    arr = temp;
}

double Erosion::calcCarryCapacity(double heightDiff, double minSlope, double velocity, double water, double capacity)
{
    return max(abs(heightDiff), minSlope) * velocity * water * capacity;
}

double Erosion::calcHeightDiff(double HeightOld, double HeightNew)
{
    return HeightNew - HeightOld;
}

double Erosion::calcVelocity(double velocity, double HeightDiff, double gravity)
{
    return  sqrt(velocity * velocity + HeightDiff * gravity);
}

double Erosion::calcWater(double water, double evaporation)
{
    return  water * (1 - evaporation);
}

Erosion::gradient Erosion::calcGradient(double posX, double posY)
{
    gradient g;
    int coordX = static_cast<int>(posX);
    int coordY = static_cast<int>(posY);

    double x = posX - coordX;
    double y = posY - coordY;

    double heightNW, heightNE, heightSW, heightSE;

    heightNW = grid->GetVal(coordX - 1, coordY - 1);
    heightNE = grid->GetVal(coordX + 1, coordY - 1);
    heightSW = grid->GetVal(coordX - 1, coordY + 1);
    heightSE = grid->GetVal(coordX + 1, coordY + 1);

    double gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
    double gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

    double height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;
	
    g.gradientX = gradientX;
    g.gradientY = gradientY;
    g.Height = height;

    return g;
}

Erosion::dropletMove Erosion::CalcDropletMove(Erosion::gradient g)
{
    dropletMove p;
    double dirX = 0;
    double dirY = 0;
    dirX = dirX * config->inertia - g.gradientX * (1 - config->inertia);
    dirY = dirY * config->inertia - g.gradientY * (1 - config->inertia);
    double length = sqrt(dirX * dirX + dirY * dirY);
    dirX = dirX / length / config->stepFac;
    dirY = dirY / length / config->stepFac;
    p.gradientx = g.gradientX;
    p.gradienty = g.gradientY;
    p.x = dirX;
    p.y = dirY;
    p.length = length;
    return p;
}

Erosion::dropletPoint Erosion::CalcNewDropletPoint(double cx, double cy)
{
    dropletPoint pr;
    gradient g = calcGradient(cx, cy);
    dropletMove p = CalcDropletMove(g);

    int gridx = cx;
    int gridy = cy;

    double cellOffsetX = cx - gridx;
    double cellOffsetY = cy - gridy;
    cx = p.x + cx;
    cy = p.y + cy;

    double newHeight = calcGradient(cx, cy).Height;

    double deltaHeight = calcHeightDiff(g.Height, newHeight);

    pr.x = cx;
    pr.y = cy;
    pr.length = p.length;
    pr.deltaHeight = deltaHeight;
    pr.newHeight = newHeight;
    pr.gradientx = p.gradientx;
    pr.gradienty = p.gradienty;
    return  pr;
}

void Erosion::PlaceSediment(double amountToDeposit, int gridx, int gridy, double cellOffsetX, double cellOffsetY, double min, double max, double current)
{
    grid->AddVal(gridx, gridy, amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY) * (min / current));
    grid->AddVal(gridx + 1, gridy, amountToDeposit * cellOffsetX * (1 - cellOffsetY) * (min / grid->GetVal(gridx + 1, gridy)));
    grid->AddVal(gridx, gridy + 1, amountToDeposit * (1 - cellOffsetX) * cellOffsetY * (min / grid->GetVal(gridx, gridy + 1)));
    grid->AddVal(gridx + 1, gridy + 1, amountToDeposit * cellOffsetX * cellOffsetY * (min / grid->GetVal(gridx + 1, gridy + 1)));
}

//void Erosion::DrawPath(sf::RenderWindow &window, double mx, double my, int cellsize)
//{
//    double erodeSpeed = config->erodeSpeed;
//    sf::Vertex line[] =
//    {
//        sf::Vertex(sf::Vector2f(10.f, 10.f)),
//        sf::Vertex(sf::Vector2f(150.f, 150.f))
//    };
//    line[0].color.r = 255;
//    line[0].color.g = 0;
//    line[0].color.b = 0;
//    line[1].color.r = 255;
//    line[1].color.g = 0;
//    line[1].color.b = 0;
//
//    double cx = mx / cellsize;
//    double cy = my / cellsize;
//    int i = 0;
//
//    double speed = initialSpeed;
//    double water = initialWaterVolume;
//    double sediment = 0;
//    float oldgx = 0;
//    float oldgy = 0;
//
//    while (i< config->maxDropletLifetime)
//    {
//        if ((int)cx > 0 && (int)cx < width - 1 && (int)cy > 0 && (int)cy < height - 1)
//        {
//        	i++;
//            gradient g = calcGradient(cx, cy);
//
//
//            double dirX = 0;
//            double dirY = 0;
//            dirX = (dirX * config->inertia - g.gradientX * (1 - config->inertia));
//            dirY = (dirY * config->inertia - g.gradientY * (1 - config->inertia));
//            double length = sqrt(dirX * dirX + dirY * dirY);
//            dirX = dirX / length / config->stepFac;
//            dirY = dirY / length / config->stepFac;
//            line[0].position.x = cx * cellsize;
//            line[0].position.y = cy * cellsize;
//            cx = dirX + cx;
//            cy = dirY + cy;
//            line[1].position.x = cx*cellsize;
//            line[1].position.y = cy*cellsize;
//
//            if(isnan(cx) || isnan(cy))
//                return;
//
//            double newHeight = calcGradient(cx, cy).Height;
//            double deltaHeight = calcHeightDiff(g.Height, newHeight);
//            
//
//            if (isnan(speed))
//                speed = 0.1;
//
//            double sedimentCapacity = calcCarryCapacity(deltaHeight, config->minSlope, speed, water, config->sedimentCapacityFactor);
//            double loose_sediment = ((oldgx - g.gradientX) + (oldgy - g.gradientY)) / (g.gradientX + g.gradientY);
//            oldgx = g.gradientX;
//            oldgy = g.gradientY;
//
//            double cellOffsetX = cx - (int)cx;
//            double cellOffsetY = cy - (int)cy;
//            if (!isnan(cx) && !isnan(cy))
//            {
//                if(abs(loose_sediment) > 1)
//                {
//                    erodeSpeed /= 2;
//                }
//                if (sediment > sedimentCapacity || deltaHeight > 0) {
//                    // If moving uphill (deltaHeight > 0) try fill up to the current height, otherwise deposit a fraction of the excess sediment
//                    double amountToDeposit = (deltaHeight > 0) ? min(deltaHeight, sediment) : (sediment - sedimentCapacity) * config->depositSpeed;
//                    sediment -= amountToDeposit;
//                    line[0].color.r = 255;
//                    line[1].color.r = 255;
//                    line[0].color.b = 0;
//                    line[1].color.b = 0;
//                }
//                else
//                {
//                    float amountToErode = min((sedimentCapacity - sediment) * erodeSpeed, -deltaHeight);
//                    sediment += amountToErode;
//                    line[0].color.b = 255;
//                    line[1].color.b = 255;
//                    line[0].color.r = 0;
//                    line[1].color.r = 0;
//                }
//            }
//
//            speed = calcVelocity(speed, deltaHeight, config->gravity);
//            water = calcWater(water, config->evaporateSpeed);
//
//            window.draw(line, 2, sf::Lines);
//            if(length<0.002)
//                break;
//        }
//        else { break; }
//    }
//}

void Erosion::Erode(double mx, double my) {
    double erodeSpeed = config->erodeSpeed;
    double cx = mx;
    double cy = my;
    int i = 0;

    double speed = initialSpeed;
    double water = initialWaterVolume;
    double sediment = 0;
    double minLength = 0.002;
    double oldgx = 0;
    double oldgy = 0;

    while (i < config->maxDropletLifetime)
    {
        int cx_int = static_cast<int>(cx);
        int cy_int = static_cast<int>(cy);

        if (cx_int > 0 && cx_int < width - 1 && cy_int > 0 && cy_int < height - 1 && !isnan(cx) && !isnan(cy))
        {
            i++;
            double cellOffsetX = cx - cx_int;
            double cellOffsetY = cy - cy_int;
            dropletPoint p = CalcNewDropletPoint(cx, cy);
            cx = p.x;
            cy = p.y;

            double deltaHeight = p.deltaHeight;
            if (isnan(speed)) speed = 0.0001;

            double sedimentCapacity = calcCarryCapacity(deltaHeight, config->minSlope, speed, water, config->sedimentCapacityFactor);

            double loose_sediment = ((oldgx - p.gradientx) + (oldgy - p.gradienty)) / (p.gradientx + p.gradienty);

            oldgx = p.gradientx;
            oldgy = p.gradienty;

            if (!isnan(cx) && !isnan(cy))
            {
                double minimum = min(min(grid->GetVal(cx_int, cy_int), grid->GetVal(cx_int + 1, cy_int)), min(grid->GetVal(cx_int, cy_int + 1), grid->GetVal(cx_int + 1, cy_int + 1)));
                double maximum = max(max(grid->GetVal(cx_int, cy_int), grid->GetVal(cx_int + 1, cy_int)), max(grid->GetVal(cx_int, cy_int + 1), grid->GetVal(cx_int + 1, cy_int + 1)));
                double current = grid->GetVal(cx_int, cy_int);


                if (abs(loose_sediment) > 1.0)
                {
                    PlaceSediment(abs(sediment)/38.0, cx_int, cy_int, cellOffsetX, cellOffsetY, minimum, maximum, current);  
                    //sediment = 0;
                    //erodeSpeed /= 2.0;
                    return;
                }

                if (p.length < minLength || i >= config->maxDropletLifetime - 1)
                {
                    grid->Brush(cx_int, cy_int, 2, -abs(sediment)/38.0);

                    return;
                }

                if (sediment > sedimentCapacity || deltaHeight > 0) {
                    // If moving uphill (deltaHeight > 0) try fill up to the current height, otherwise deposit a fraction of the excess sediment
                    double amountToDeposit = (deltaHeight > 0)
                        ? min(deltaHeight, sediment)
                        : (sediment - sedimentCapacity) * config->depositSpeed;
                    amountToDeposit = abs(amountToDeposit);
                    sediment -= amountToDeposit;

                    PlaceSediment(amountToDeposit/38.0, cx_int, cy_int, cellOffsetX, cellOffsetY, minimum, maximum, current);
                }
                else
                {
                    double amountToErode = min((sedimentCapacity - sediment) * erodeSpeed, -deltaHeight);
                    amountToErode = abs(amountToErode);
                    sediment += amountToErode;

                    if (!isnan(amountToErode)) {
                        //std::cout << amountToErode << std::endl;
                        grid->Brush(cx_int, cy_int, 2, amountToErode/38.0);
                    }
                }

                speed = calcVelocity(speed, deltaHeight, config->gravity);
                water = calcWater(water, config->evaporateSpeed);
            }
        }
        else
        {
            grid->Brush(cx_int, cy_int, 2, -abs(sediment) / 38.0);
	        return;
        }
    }
}

void Erosion::Erode(int iter) {
    std::uniform_real_distribution<double> unifx(1,width-1);
    std::uniform_real_distribution<double> unify(1, height-1);
    for (int i = 0; i < iter; i++)
    {
        double rx = unifx(re);
        double ry = unify(re);

        Erode(rx, ry);
    }
}

Erosion::Erosion(int pwidth, int pheight, int iter, Config pconfig)
{
    grid = new Grid(pwidth, pheight);
    width = pwidth;
    height = pheight;
    config = &pconfig;

    re.seed(time(nullptr));
    std::uniform_real_distribution<double> unif(0, 1000);
    double r = unif(re);
    double fac = 0.005;
    PerlinNoise pn(time(nullptr));

    for (int i = 0; i < pwidth; i++)
    {
        for (int j = 0; j < pheight; j++)
        {
            double n = pn.noise(i * fac, j * fac, r);
            n += pn.noise(i * fac * 2, j * fac * 2, r + 10) * 0.5;
            n += pn.noise(i * fac * 4, j * fac * 4, r + 20) * 0.125;
            n += pn.noise(i * fac * 8, j * fac * 8, r + 30) * 0.125;
            grid->SetVal(i, j, n);
        }
    }
    grid->Normalize();
    if(iter <= 0)
        return;
    Erode(iter);
}
