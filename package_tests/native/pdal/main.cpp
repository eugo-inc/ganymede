/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pdal: core API; calls pdal::Config::fullVersionString().
 */

#include <pdal/PipelineManager.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/pdal_config.hpp>

#include <iostream>
#include <sstream>
#include <string>

static pdal::point_count_t runPipeline(const std::string& pipelineJson)
{
    pdal::PipelineManager manager;
    std::istringstream input(pipelineJson);
    manager.readPipeline(input);
    return manager.execute();
}

int main()
{
    const pdal::point_count_t statsCount = runPipeline(R"json({ "pipeline": [
        { "type": "readers.faux", "mode": "ramp", "count": 1000, "bounds": "([0, 9], [0, 9], [0, 9])" },
        { "type": "filters.stats", "dimensions": "X,Y,Z" }
    ] })json");
    if (statsCount != 1000)
    {
        std::cerr << "pdal: the stats pipeline produced " << statsCount << " points, expected 1000\n";
        return 1;
    }

    const pdal::point_count_t poissonCount = runPipeline(R"json({ "pipeline": [
        { "type": "readers.faux", "mode": "random", "count": 2000, "bounds": "([0, 10], [0, 10], [0, 1])" },
        { "type": "filters.poisson", "depth": 6 }
    ] })json");
    if (poissonCount == 0)
    {
        std::cerr << "pdal: filters.poisson produced no points\n";
        return 3;
    }

    const pdal::point_count_t csfCount = runPipeline(R"json({ "pipeline": [
        { "type": "readers.faux", "mode": "random", "count": 2000, "bounds": "([0, 50], [0, 50], [0, 2])" },
        { "type": "filters.csf" }
    ] })json");
    if (csfCount != 2000)
    {
        std::cerr << "pdal: filters.csf produced " << csfCount << " points, expected 2000\n";
        return 4;
    }

    pdal::StageFactory factory;
    for (const char* stageName : { "readers.arrow", "writers.arrow", "readers.hdf", "readers.icebridge",
                                   "readers.pgpointcloud", "writers.pgpointcloud" })
    {
        if (factory.createStage(stageName) == nullptr)
        {
            std::cerr << "pdal: the stage " << stageName << " could not be created; its plugin did not load\n";
            return 2;
        }
    }

    std::cout << "pdal " << pdal::Config::fullVersionString() << ": 1000 points through filters.stats, " << poissonCount
              << " through filters.poisson, " << csfCount << " through filters.csf, 6 plugin stages loaded\n";
    return 0;
}
