/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "HashedSetAbstractDomain.h"

using Domain = HashedSetAbstractDomain<std::string>;

TEST(HashedSetAbstractDomainTest, latticeOperations) {
  Domain e1("a");
  Domain e2({"a", "b", "c"});
  Domain e3({"b", "c", "d"});

  EXPECT_THAT(e1.elements(), ::testing::UnorderedElementsAre("a"));
  EXPECT_THAT(e2.elements(), ::testing::UnorderedElementsAre("a", "b", "c"));
  EXPECT_THAT(e3.elements(), ::testing::UnorderedElementsAre("b", "c", "d"));

  std::ostringstream out;
  out << e1;
  EXPECT_EQ("[#1]{a}", out.str());

  EXPECT_TRUE(Domain::bottom().leq(Domain::top()));
  EXPECT_FALSE(Domain::top().leq(Domain::bottom()));
  EXPECT_FALSE(e2.is_top());
  EXPECT_FALSE(e2.is_bottom());

  EXPECT_TRUE(e1.leq(e2));
  EXPECT_FALSE(e1.leq(e3));
  EXPECT_TRUE(e2.equals(Domain({"b", "c", "a"})));
  EXPECT_FALSE(e2.equals(e3));

  EXPECT_THAT(e2.join(e3).elements(),
              ::testing::UnorderedElementsAre("a", "b", "c", "d"));
  EXPECT_TRUE(e1.join(e2).equals(e2));
  EXPECT_TRUE(e2.join(Domain::bottom()).equals(e2));
  EXPECT_TRUE(e2.join(Domain::top()).is_top());
  EXPECT_TRUE(e1.widening(e2).equals(e2));

  EXPECT_THAT(e2.meet(e3).elements(),
              ::testing::UnorderedElementsAre("b", "c"));
  EXPECT_TRUE(e1.meet(e2).equals(e1));
  EXPECT_TRUE(e2.meet(Domain::bottom()).is_bottom());
  EXPECT_TRUE(e2.meet(Domain::top()).equals(e2));
  EXPECT_FALSE(e1.meet(e3).is_bottom());
  EXPECT_TRUE(e1.meet(e3).elements().empty());
  EXPECT_TRUE(e1.narrowing(e2).equals(e1));

  EXPECT_TRUE(e2.contains("a"));
  EXPECT_FALSE(e3.contains("a"));

  // Making sure no side effect happened.
  EXPECT_THAT(e1.elements(), ::testing::UnorderedElementsAre("a"));
  EXPECT_THAT(e2.elements(), ::testing::UnorderedElementsAre("a", "b", "c"));
  EXPECT_THAT(e3.elements(), ::testing::UnorderedElementsAre("b", "c", "d"));
}

TEST(HashedSetAbstractDomainTest, destructiveOperations) {
  Domain e1("a");
  Domain e2({"a", "b", "c"});
  Domain e3({"b", "c", "d"});

  e1.add("b");
  EXPECT_THAT(e1.elements(), ::testing::UnorderedElementsAre("a", "b"));
  e1.add({"a", "c"});
  EXPECT_TRUE(e1.equals(e2));
  std::vector<std::string> v1 = {"a", "b"};
  e1.add(v1.begin(), v1.end());
  EXPECT_TRUE(e1.equals(e2));

  e1.remove("b");
  EXPECT_THAT(e1.elements(), ::testing::UnorderedElementsAre("a", "c"));
  e1.remove("d");
  EXPECT_THAT(e1.elements(), ::testing::UnorderedElementsAre("a", "c"));
  std::vector<std::string> v2 = {"a", "e"};
  e1.remove(v2.begin(), v2.end());
  EXPECT_THAT(e1.elements(), ::testing::UnorderedElementsAre("c"));
  e1.remove({"a", "c"});
  EXPECT_TRUE(e1.elements().empty());

  e1.join_with(e2);
  EXPECT_THAT(e1.elements(), ::testing::UnorderedElementsAre("a", "b", "c"));
  e1.join_with(Domain::bottom());
  EXPECT_TRUE(e1.equals(e2));
  e1.join_with(Domain::top());
  EXPECT_TRUE(e1.is_top());

  e1 = Domain("a");
  e1.widen_with(Domain({"b", "c"}));
  EXPECT_TRUE(e1.equals(e2));

  e1 = Domain("a");
  e2.meet_with(e3);
  EXPECT_THAT(e2.elements(), ::testing::UnorderedElementsAre("b", "c"));
  e1.meet_with(e2);
  EXPECT_TRUE(e1.elements().empty());
  e1.meet_with(Domain::top());
  EXPECT_THAT(e2.elements(), ::testing::UnorderedElementsAre("b", "c"));
  e1.meet_with(Domain::bottom());
  EXPECT_TRUE(e1.is_bottom());

  e1 = Domain("a");
  e1.narrow_with(Domain({"a", "b"}));
  EXPECT_THAT(e1.elements(), ::testing::UnorderedElementsAre("a"));

  EXPECT_FALSE(e2.is_top());
  e1.set_to_top();
  EXPECT_TRUE(e1.is_top());
  e1.set_to_bottom();
  EXPECT_TRUE(e1.is_bottom());
  EXPECT_FALSE(e2.is_bottom());
  e2.set_to_bottom();
  EXPECT_TRUE(e2.is_bottom());

  e1 = Domain({"a", "b", "c", "d"});
  e2 = e1;
  EXPECT_TRUE(e1.equals(e2));
  EXPECT_TRUE(e2.equals(e1));
  EXPECT_FALSE(e2.is_bottom());
  EXPECT_THAT(e2.elements(),
              ::testing::UnorderedElementsAre("a", "b", "c", "d"));
}
