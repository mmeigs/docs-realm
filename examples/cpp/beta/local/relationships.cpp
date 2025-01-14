#include <catch2/catch_test_macros.hpp>
#include <cpprealm/sdk.hpp>
#include <cpprealm/experimental/sdk.hpp>

// :replace-start: {
//   "terms": {
//     "Beta_Relationship_": "",
//     "Beta_Inverse_": ""
//   }
// }

using namespace realm::experimental;

// :snippet-start: beta-to-one-relationship
struct Beta_Relationship_FavoriteToy {
    primary_key<realm::uuid> _id;
    std::string name;
};
REALM_SCHEMA(Beta_Relationship_FavoriteToy, _id, name)

struct Beta_Relationship_Dog {
    primary_key<realm::uuid> _id;
    std::string name;
    int64_t age;
    
    // Define a relationship as a link to another Realm object
    Beta_Relationship_FavoriteToy* favoriteToy;
};
REALM_SCHEMA(Beta_Relationship_Dog, _id, name, age, favoriteToy)
// :snippet-end:

// :snippet-start: beta-define-inverse-relationship
struct Beta_Inverse_Dog;
struct Beta_Inverse_Person {
    primary_key<int64_t> _id;
    std::string name;
    int64_t age = 0;
    Beta_Inverse_Dog* dog;
};
REALM_SCHEMA(Beta_Inverse_Person, _id, name, age, dog)
struct Beta_Inverse_Dog {
    primary_key<int64_t> _id;
    std::string name;

    int64_t age = 0;
    linking_objects<&Beta_Inverse_Person::dog> owners;
};
REALM_SCHEMA(Beta_Inverse_Dog, _id, name, age, owners)
// :snippet-end:

struct Beta_Relationship_Employee {
    int64_t _id;
    std::string firstName;
    std::string lastName;
};
REALM_SCHEMA(Beta_Relationship_Employee, _id, firstName, lastName)

// :snippet-start: beta-to-many-relationship
struct Beta_Relationship_Company {
    int64_t _id;
    std::string name;
    // To-many relationships are a list, represented here as a
    // vector container whose value type is the Realm object
    // type that the list field links to.
    std::vector<Beta_Relationship_Employee*> employees;
};
REALM_SCHEMA(Beta_Relationship_Company, _id, name, employees)
// :snippet-end:

TEST_CASE("Beta define to-one relationship example", "[write]") {
    auto relative_realm_path_directory = "beta_relationship/";
    std::filesystem::create_directories(relative_realm_path_directory);
    std::filesystem::path path = std::filesystem::current_path().append(relative_realm_path_directory);
    path = path.append("dog_objects");
    path = path.replace_extension("realm");
    // :snippet-start: beta-create-to-one-relationship
    auto config = realm::db_config();
    config.set_path(path); // :remove:
    auto realmInstance = db(std::move(config));
    
    auto favoriteToy = Beta_Relationship_FavoriteToy {
        ._id = realm::uuid("68b696c9-320b-4402-a412-d9cee10fc6a5"),
        .name = "Wubba"
    };
    
    auto dog = Beta_Relationship_Dog {
        ._id = realm::uuid("68b696d7-320b-4402-a412-d9cee10fc6a3"),
        .name = "Lita",
        .age = 10,
        .favoriteToy = &favoriteToy
    };
    
    realmInstance.write([&] {
        realmInstance.add(std::move(dog));
    });
    // :snippet-end:
    
    auto managedDogs = realmInstance.objects<Beta_Relationship_Dog>();
    auto specificDog = managedDogs[0];
    REQUIRE(specificDog._id == realm::uuid("68b696d7-320b-4402-a412-d9cee10fc6a3"));
    REQUIRE(specificDog.name == "Lita");
    REQUIRE(specificDog.age == static_cast<long long>(10));
    REQUIRE(specificDog.favoriteToy->name == "Wubba");
    REQUIRE(managedDogs.size() == 1);

    realmInstance.write([&] {
        realmInstance.remove(specificDog);
    });

    auto managedDogsAfterDelete = realmInstance.objects<Beta_Relationship_Dog>();
    REQUIRE(managedDogsAfterDelete.size() == 0);
}

TEST_CASE("Beta define to-many relationship example", "[write]") {
    auto relative_realm_path_directory = "beta_relationship/";
    std::filesystem::create_directories(relative_realm_path_directory);
    std::filesystem::path path = std::filesystem::current_path().append(relative_realm_path_directory);
    path = path.append("dog_objects");
    path = path.replace_extension("realm");
    // :snippet-start: beta-create-to-many-relationship
    auto config = realm::db_config();
    config.set_path(path); // :remove:
    auto realmInstance = db(std::move(config));
    
    auto employee1 = Beta_Relationship_Employee {
        ._id = 23456,
        .firstName = "Pam",
        .lastName = "Beesly"
    };
    
    auto employee2 = Beta_Relationship_Employee {
        ._id = 34567,
        .firstName = "Jim",
        .lastName = "Halpert"
    };
    
    auto company = Beta_Relationship_Company {
        ._id = 45678,
        .name = "Dunder Mifflin"
    };
    
    // Use the `push_back` member function available to the
    // `ListObjectPersistable<T>` template to append `Employee` objects to
    // the `Company` `employees` list property.
    company.employees.push_back(&employee1);
    company.employees.push_back(&employee2);
    
    realmInstance.write([&] {
        realmInstance.add(std::move(company));
    });
    // :snippet-end:
    
    auto companies = realmInstance.objects<Beta_Relationship_Company>();
    auto namedDunderMifflin = companies.where([](auto &company) {
        return company.name == "Dunder Mifflin";
    });
    CHECK(namedDunderMifflin.size() >= 1);
    auto dunderMifflin = namedDunderMifflin[0];
    
    SECTION("Verify expected properties of retrieved object") {
        REQUIRE(dunderMifflin.name == "Dunder Mifflin");
        auto employeeCount = dunderMifflin.employees.size();
        REQUIRE(employeeCount >= 2);
        auto companyEmployees = dunderMifflin.employees;
        auto employees = realmInstance.objects<Beta_Relationship_Employee>();
        auto employeesNamedJim = employees.where([](auto &employee) {
            return employee.firstName == "Jim";
        });
        REQUIRE(employeesNamedJim.size() >= 1);
    }

    realmInstance.write([&] {
        realmInstance.remove(dunderMifflin);
    });

    auto managedCompaniesAfterDelete = realmInstance.objects<Beta_Relationship_Company>();
    REQUIRE(managedCompaniesAfterDelete.size() == 0);
}

TEST_CASE("Beta define inverse relationship example", "[write]") {
    auto relative_realm_path_directory = "beta_relationship/";
    std::filesystem::create_directories(relative_realm_path_directory);
    std::filesystem::path path = std::filesystem::current_path().append(relative_realm_path_directory);
    path = path.append("inverse_objects");
    path = path.replace_extension("realm");
    // :snippet-start: beta-create-inverse-relationship
    auto config = realm::db_config();
    config.set_path(path); // :remove:
    auto realm = db(std::move(config));

    auto dog = Beta_Inverse_Dog {
        .name = "Bowser"
    };

    auto [jack, jill] = realm.write([&realm]() {
        auto person = Beta_Inverse_Person {
            .name = "Jack",
            .age = 27,
            .dog = nullptr
        };
        
        Beta_Inverse_Person person2;
        person2.name = "Jill";
        person2.age = 28;
        person2.dog = nullptr;
        
        return realm.insert(std::move(person), std::move(person2));
    });
    
    CHECK(jack.dog == nullptr); // :remove:
    realm.write([&dog, jack = &jack]() {
        jack->dog = &dog;
    });
    CHECK(jack.dog->name == "Bowser"); // :remove:
    
    // After assigning `&dog` to jack's `dog` property,
    // the backlink automatically updates to reflect
    // the inverse relationship through the dog's `owners`
    // property
    CHECK(jack.dog->owners.size() == 1);
    
    realm.write([&dog, jill = &jill]() {
        jill->dog = &dog;
    });
    CHECK(jill.dog->name == "Bowser"); // :remove:
    
    // After assigning the same `&dog` to jill's `dog`
    // property, the backlink automatically updates
    CHECK(jill.dog->owners.size() == 2);
    CHECK(jack.dog->owners.size() == 2);
    
    // Removing the relationship from the parent object
    // automatically updates the inverse relationship
    realm.write([jack = &jack]() {
        jack->dog = nullptr;
    });
    CHECK(jack.dog == nullptr);
    CHECK(jill.dog->owners.size() == 1);
    // :snippet-end:
    realm.write([jill = &jill]() {
        jill->dog = nullptr;
    });
    CHECK(realm.objects<Beta_Inverse_Dog>()[0].owners.size() == 0);
}

TEST_CASE("Beta update inverse relationship example", "[write]") {
    auto relative_realm_path_directory = "beta_relationship/";
    std::filesystem::create_directories(relative_realm_path_directory);
    std::filesystem::path path = std::filesystem::current_path().append(relative_realm_path_directory);
    path = path.append("inverse_objects");
    path = path.replace_extension("realm");
    // :snippet-start: beta-update-inverse-relationship
    auto config = realm::db_config();
    config.set_path(path); // :remove:
    auto realm = db(std::move(config));

    auto dog = Beta_Inverse_Dog {
        .name = "Wishbone"
    };

    auto [joe] = realm.write([&realm]() {
        auto person = Beta_Inverse_Person {
            .name = "Joe",
            .age = 27,
            .dog = nullptr
        };
        return realm.insert(std::move(person));
    });
    
    CHECK(joe.dog == nullptr); // :remove:
    realm.write([&dog, joe = &joe]() {
        joe->dog = &dog;
    });
    CHECK(joe.dog->name == "Wishbone"); // :remove:
    
    // After assigning `&dog` to jack's `dog` property,
    // the backlink automatically updates to reflect
    // the inverse relationship through the dog's `owners`
    // property.
    CHECK(joe.dog->owners.size() == 1);
    // :snippet-end:
    realm.write([joe = &joe]() {
        joe->dog = nullptr;
    });
    CHECK(joe.dog == nullptr);
    CHECK(realm.objects<Beta_Inverse_Dog>()[0].owners.size() == 0);
}

TEST_CASE("Beta delete inverse relationship example", "[write]") {
    auto relative_realm_path_directory = "beta_relationship/";
    std::filesystem::create_directories(relative_realm_path_directory);
    std::filesystem::path path = std::filesystem::current_path().append(relative_realm_path_directory);
    path = path.append("inverse_objects");
    path = path.replace_extension("realm");
    // :snippet-start: beta-delete-inverse-relationship
    auto config = realm::db_config();
    config.set_path(path); // :remove:
    auto realm = db(std::move(config));

    auto dog = Beta_Inverse_Dog {
        .name = "Wishbone"
    };

    auto [joe] = realm.write([&realm]() {
        auto person = Beta_Inverse_Person {
            .name = "Joe",
            .age = 27,
            .dog = nullptr
        };
        return realm.insert(std::move(person));
    });
    
    CHECK(joe.dog == nullptr); // :remove:
    
    // Assign an object with an inverse relationship
    // to automatically set the value of the inverse relationship
    realm.write([&dog, joe = &joe]() {
        joe->dog = &dog;
    });
    CHECK(joe.dog->name == "Wishbone"); // :remove:
    CHECK(joe.dog->owners.size() == 1);
    
    // ... Later ...
    
    // Removing the relationship from the parent object
    // automatically updates the inverse relationship
    realm.write([joe = &joe]() {
        joe->dog = nullptr;
    });
    CHECK(joe.dog == nullptr); // :remove:
    CHECK(realm.objects<Beta_Inverse_Dog>()[0].owners.size() == 0);
    // :snippet-end:
}
// :replace-end:
