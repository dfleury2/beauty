#include <beauty/router.hpp>

#include <algorithm>

namespace beauty {
// --------------------------------------------------------------------------
void
router::add_route(beast::http::verb v, route&& r)
{
    _routes[v].push_back(std::move(r));

    std::sort(_routes[v].begin(), _routes[v].end(),
            [](const beauty::route& lh, const beauty::route& rh) {
                const auto lh_segment_count = lh.segments().size();
                const auto rh_segment_count = rh.segments().size();

                if (lh_segment_count < rh_segment_count) return true;
                if (lh_segment_count > rh_segment_count) return false;

                int lh_cost = 0, rh_cost = 0;
                int power = 1;
                for (int i = lh_segment_count - 1; i >= 0; --i) {
                    if (lh.segments()[i][0] == ':') lh_cost += power;
                    if (rh.segments()[i][0] == ':') rh_cost += power;
                    power *= 2;
                }

                return lh_cost < rh_cost;
            });
}

}
