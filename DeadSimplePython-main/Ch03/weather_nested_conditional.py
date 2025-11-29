class Special:
    TODAY = 'lasagna'

lunch_order = input("What would you like for lunch? ")

match lunch_order:
    case Special.TODAY:
        print("Today's special is awesome!")
    case 'pizza':
        print("Pizza time!")
    case 'sandwich':
        print("Here's your sandwich")
    case 'salad' | 'soup':  # 多个选项
        print("Eating healthy, eh?")
    case ice_cream if 'ice cream' in ice_cream:  # 守卫条件
        flavor = ice_cream.replace('ice cream', '').strip()
        print(f"Here's your {flavor} ice cream.")
    case order:  # 默认情况 (捕获所有)
        print(f"Enjoy your {order}.")