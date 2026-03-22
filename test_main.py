from main import greet, get_age


def test_greet():
    assert greet() == "Hello Dima"



def test_get_age():
    assert get_age() == 252