//******************************************************************************
//* File:   TestFrame.cpp
//* Author: Jon Newman <jpnewman snail mit dot edu>
//*
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
//******************************************************************************

#include "TestFrame.h"

#include <thread>

#include <cpptoml.h>

#include "../../lib/utility/IOFormat.h"
#include "../../lib/utility/TOMLSanitize.h"

namespace oat {

TestFrame::TestFrame(const std::string &sink_address)
: FrameServer(sink_address)
{
    // Initialize time
    tick_ = clock_.now();
}

po::options_description TestFrame::options() const
{
    // Update CLI options
    po::options_description local_opts;
    local_opts.add_options()
        ("test-image,f", po::value<std::string>(),
         "Path to test image used as frame source.")
        ("color,C", po::value<std::string>(),
         "Pixel color format. Defaults to BGR.\n"
         "Values:\n"
         "  mono: \t 8-bit Greyscale image.\n"
         "  bgr: \t8-bit, 3-chanel, BGR Color image.\n")
        ("fps,r", po::value<double>(),
         "Frames to serve per second.")
        ("num-frames,n", po::value<uint64_t>(),
         "Number of frames to serve before exiting.")
        ;

    return local_opts;
}

void TestFrame::applyConfiguration(const po::variables_map &vm,
                                   const config::OptionTable &config_table)
{
    // Test image path
    oat::config::getValue(vm, config_table, "test-image", file_name_, true);

    // Pixel color
    std::string col_str;
    if (oat::config::getValue<std::string>(vm, config_table, "color", col_str))
        color_ = oat::Pixel::color(col_str);

    // Number of frames to serve
    oat::config::getNumericValue<uint64_t>(
        vm, config_table, "num-frames", num_samples_, 1);

    // Frame rate
    double fps;
    if (oat::config::getNumericValue(vm, config_table, "fps", fps, 0.0))
        frame_period_ = Token::Seconds(1.0 / fps);
}

bool TestFrame::connectToNode() {

    auto mat = cv::imread(file_name_, oat::Pixel::cvImreadCode(color_));

    if (mat.data == NULL)
        throw (std::runtime_error("File \"" + file_name_ + "\" could not be read."));

    frame_sink_.reserve(mat.total() * mat.elemSize());
    frame_sink_.bind(frame_period_, mat.rows, mat.cols, color_);

    // Static image, never changes
    shared_frame_ = frame_sink_.retrieve();
    shared_frame_->copyFrom(mat);

    // Setup sample rate info on internal copy
    shared_frame_->set_period(frame_period_);

    return true;
}

int TestFrame::process()
{
    if (shared_frame_->tick() < num_samples_) {

        // START CRITICAL SECTION //
        ////////////////////////////

        // Wait for sources to read
        frame_sink_.wait();

        // Zero frame copy
        shared_frame_->incrementCount();

        // Tell sources there is new data
        frame_sink_.post();

        ////////////////////////////
        //  END CRITICAL SECTION  //

        std::this_thread::sleep_for(frame_period_ - (clock_.now() - tick_));
        tick_ = clock_.now();

        return 0;
    }

    return 1;
}

} /* namespace oat */
