def tell_joke(joke_type):
    if joke_type == "funny":
        print("How can you tell an elephant is in your fridge?")
        print("There are footprints in the butter!")
    elif joke_type == "lethal":
        print("Wenn ist das Nunstück git und Slotermeyer?")
        print("Ja! Beiherhund das Oder die Flipperwaldt gersput!")
    else:
        print("Why did the chicken cross the road?")
        print("To get to the other side!")


tell_joke("funny")



def make_tea():
    """Will produce a concoction almost,
    but not entirely unlike tea."""
    pass

# 访问文档字符串
print(make_tea.__doc__)
help(make_tea)