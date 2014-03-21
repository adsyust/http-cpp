// Copyright 2014 by Konstantin (Kosta) Baumann
//
// Distributed under the MIT license (See accompanying file LICENSE.txt)
//

#pragma once

#include "eval_context.hpp"
#include "exception.hpp"
#include "macros.hpp"
#include "test.hpp"
#include "test_registry.hpp"
#include "test_result.hpp"
#include "test_suite_result.hpp"

namespace cute {

    inline std::string temp_folder() {
        return cute::detail::eval_context::current().create_temp_folder();
    }

    struct context {
        std::vector<std::function<void(test_result const& rep)>> reporters;
        std::string include_tags;
        std::string exclude_tags;

        inline test_suite_result run() const {
            return run(detail::test_registry::instance().tests);
        }

        inline test_suite_result run(
            std::vector<test> const& tests
        ) const {
            auto const time_start_all = detail::time_now();

            detail::eval_context eval;

            auto const incl_tags = detail::parse_tags(include_tags);
            auto const excl_tags = detail::parse_tags(exclude_tags);

            for(auto&& test : tests) {
                ++eval.test_cases;

                if(detail::skip_test(test.tags, incl_tags, excl_tags)) {
                    ++eval.test_cases_skipped;
                    continue;
                }

                auto rep = test_result(test.name, true, "", "", test.file, test.line, 0);
                auto const time_start = detail::time_now();

                try {
                    auto const count_start = eval.checks_performed.load();

                    --eval.checks_performed; // decr by one since CUTE_DETAIL_ASSERT() below will increment it again
                    CUTE_DETAIL_ASSERT(((void)test.test_case(), true), test.file, test.line, "", cute::captures(), cute::captures());

                    auto const count_end = eval.checks_performed.load();
                    if(count_start == count_end) {
                        throw cute::detail::exception("no check performed in test case", test.file, test.line, "");
                    }

                    // ensure that the temp folder can be cleared and that no file locks exists after the test case
                    if(!eval.delete_temp_folder()) {
                        throw cute::detail::exception("could not cleanup temp folder", test.file, test.line, "");
                    }

                    ++eval.test_cases_passed;
                } catch(detail::exception const& ex) {
                    rep.pass    = false;
                    rep.file    = ex.file;
                    rep.line    = ex.line;
                    rep.reason  = ex.what();
                    rep.expr    = ex.expr;

                    for(auto&& c : ex.caps.list) {
                        rep.captures.emplace_back(c);
                    }

                    // ensure that the temp folder gets also cleared even in case of a test failure
                    eval.delete_temp_folder();

                    ++eval.test_cases_failed;
                }

                auto const time_end = detail::time_now();
                rep.duration_ms = detail::time_diff_ms(time_start, time_end);

                for(auto&& reporter : reporters) {
                    if(reporter) { reporter(rep); }
                }
            }

            auto const time_end_all = detail::time_now();
            eval.duration_ms = detail::time_diff_ms(time_start_all, time_end_all);

            return eval.result();
        }

    };

} // namespace cute
