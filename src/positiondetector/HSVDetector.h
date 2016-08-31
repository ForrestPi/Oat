//******************************************************************************
//* File:   HSVDetector.h
//* Author: Jon Newman <jpnewman snail mit dot edu>
//
//* Copyright (c) Jon Newman (jpnewman snail mit dot edu)
//* All right reserved.
//* This file is part of the Oat project.
//* This is free software: you can redistribute it and/or modify
//* it under the terms of the GNU General Public License as published by
//* the Free Software Foundation, either version 3 of the License, or
//* (at your option) any later version.
//* This software is distributed in the hope that it will be useful,
//* but WITHOUT ANY WARRANTY; without even the implied warranty of
//* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//* GNU General Public License for more details.
//* You should have received a copy of the GNU General Public License
//* along with this source code.  If not, see <http://www.gnu.org/licenses/>.
//****************************************************************************

#ifndef OAT_HSVDETECTOR_H
#define	OAT_HSVDETECTOR_H

#include "OatConfig.h" // Generated by CMake

#include <string>
#include <limits>
#include <opencv2/core/mat.hpp>
#ifdef NOIMP_OAT_USE_CUDA
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaimgproc.hpp>
#endif

#include "PositionDetector.h"

namespace oat {

class Position2D;

/**
 * A color-based object position detector
 */
class HSVDetector : public PositionDetector {
public:

    /**
     * A color-based object position detector with default parameters.
     * @param frame_source_address Frame SOURCE node address
     * @param position_sink_address Position SINK node address
     */
    HSVDetector(const std::string &frame_source_address,
                const std::string &position_sink_address);

    /**
     * Perform color-based object position detection.
     * @param Frame to look for object within.
     * @param position Detected object position.
     */
    void detectPosition(cv::Mat &frame, oat::Position2D &position) override;

    void appendOptions(po::options_description &opts) override;
    void configure(const po::variables_map &vm) override;

    // Accessors (used for tuning GUI)
    void set_erode_size(int erode_px);
    void set_dilate_size(int dilate_px);
    void set_min_object_area(double value) { min_object_area_ = value; }
    void set_max_object_area(double value) { max_object_area_ = value; }

private:

    // Sizes of the erode and dilate blocks
    int erode_px_ {0}, dilate_px_ {10};
    bool erode_on_ {false}, dilate_on_ {false};

    // Internal matricies
    cv::Mat threshold_frame_, erode_element_, dilate_element_;

    // HSV threshold values
    int h_min_ {0}, h_max_ {256};
    int s_min_ {0}, s_max_ {256};
    int v_min_ {0}, v_max_ {256};
    int dummy0_ {0}, dummy1_ {100000};

    // Detect object area
    double object_area_ {0.0};
    double min_object_area_ {0.0};
    double max_object_area_ {std::numeric_limits<double>::max()};

    // Parameter tuning GUI functions and properties
    bool tuning_on_ {false};
    bool tuning_windows_created_ {false};
    const std::string tuning_image_title_;
    void tune(cv::Mat &frame, const oat::Position2D &position);
    void createTuningWindows(void);

};

// Tuning GUI callbacks
void hsvDetectorMinAreaSliderChangedCallback(int value, void *);
void hsvDetectorMaxAreaSliderChangedCallback(int value, void *);
void hsvDetectorErodeSliderChangedCallback(int value, void *);
void hsvDetectorDilateSliderChangedCallback(int value, void *);

}       /* namespace oat */
#endif	/* OAT_HSVDETECTOR_H */
