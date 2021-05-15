/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2009 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Description
    Calculates species mass fractions and thermodynamic properties
    from given Z, chi and Zeta fields

Contributors/Copyright
    2014 Hagen Müller <hagen.mueller@unibw.de> Universität der Bundeswehr München

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "timeSelector.H"
#include "tableSolver.H"

#include "fvCFD.H"
#include "rhoCombustionModel.H"
#include "IOdictionary.H"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
// #include <direct.h>  
#include <stdio.h> 
using namespace std;
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    timeSelector::addOptions();
    #include "addRegionOption.H"
    argList::addOption
    (
        "fields",
        "list",
        "specify a list of fields to be reconstructed. Eg, '(U T p)' - "
        "regular expressions not currently supported"
    );

    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"

    instantList timeDirs = timeSelector::select0(runTime, args);

    HashSet<word> selectedFields;
    if (args.optionFound("fields"))
    {
        args.optionLookup("fields")() >> selectedFields;
    }

    Info<< "Creating combustion model\n" << endl;

    autoPtr<combustionModels::rhoCombustionModel> combustion
    (
        combustionModels::rhoCombustionModel::New
        (
            mesh
        )
    );

    rhoReactionThermo& thermo = combustion->thermo();    

    const IOdictionary combProps
    (
        IOobject
        (
            "combustionProperties",
            mesh.time().constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );

    const IOdictionary tableProps
    (
        IOobject
        (
            "tableProperties",
            mesh.time().constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );

    wordList tableNames(thermo.composition().species());
	tableNames.append("PVmajor");
	tableNames.append("OMGmajor");
	tableNames.append("he");
    tableNames.append("T");
    Foam::combustionModels::tableSolver solver(Foam::combustionModels::tableSolver(mesh, tableNames));

    char *curPwd = (char*)malloc(100);
    getcwd(curPwd, 100);
    std::string casePath = curPwd;
    // 读文件
    const std::string readPath = casePath+"/canteraTables/1.csv";
    ifstream inFile(readPath.c_str(), ios::in);
    if (!inFile)
    {
        Info << "打开文件失败！" << endl;
        exit(1);
    }
    int i = 0;
    std::string line;
    std::vector<double> Z,PV;
    int Zpos = 0, PVpos = 0;
    // 1. 处理数据头
    getline(inFile, line);
    std::string tmp;
    istringstream sin(line);
    int pos = 0;
    while(getline(sin, tmp, ','))
    {
        if(tmp == "Z")
            Zpos = pos;
        if(tmp == "PV")
            PVpos = pos;
        pos++;
    }
    Info << "Zpos is " << Zpos <<endl;
    Info << "PVpos is " << PVpos <<endl;

    while (getline(inFile, line))//getline(inFile, line)表示按行读取CSV文件中的数据
    {
        std::string field;
        istringstream sin(line); //将整行字符串line读入到字符串流sin中
        int n = 0;
        while(getline(sin, field, ','))//将字符串流sin中的字符读入到field字符串中，以逗号为分隔符 
        {
            if(n == Zpos) 
                Z.push_back(atof(field.c_str()));   // 将刚刚读取的字符串转换成float
            if(n == PVpos)
                PV.push_back(atof(field.c_str()));
            n++;
        }
        i++;
    }
    inFile.close();
    Info << "共读取了：" << i << "行" << endl;
    Info << "读取数据完成" << endl;

    scalarList x(3);
    List<List<int> > ubIF(Z.size());
    List<scalarList> posIF(Z.size());

    std::vector<double> T(Z.size());

    // Interpolate for  Field
    for(unsigned j = 0; j < Z.size(); ++j)
    {
        x[0] = 0;       //限定varZ的范围不超过Z(Z-1)，起始边界网格的方差不超过在Z（Z-1）
        x[1] = Z[j];                                            //对于混合分数，直接将其赋值给起始边界网格
        x[2] = PV[j];
        Info << "Z 是 " << Z[j] << "  " << "PV 是 " << PV[j] << endl;
        ubIF[j] = solver.upperBounds(x);
        posIF[j] = solver.position(ubIF[j], x);

        Info << "第 " << j << " 次读取" << endl;
        T[j] = solver.interpolate(ubIF[j], posIF[j], (solver.sizeTableNames() - 1));
        Info << "读取的温度是" << T[j] << endl;
    }

    // 写文件
    Info << "开始写入数据" << endl;
    const std::string writePath = casePath+"/outPutTables/Data.csv";
    ofstream outFile(writePath.c_str(), ios::out);
    //ios::out：如果没有文件，那么生成空文件；如果有文件，清空文件
    if (!outFile)
    {
        Info << "打开文件失败！" << endl;
        exit(1);
    }
    outFile << "Z" << ',' << "PV" << ',' << "T" << ',' << endl;
    for(unsigned j = 0; j < Z.size(); ++j)
    {
        outFile << Z[j] << ',' << PV[j] << ',' << T[j] << endl;
    }
    outFile.close();
    Info << "写入数据完成" << endl;
	return 0;
}
