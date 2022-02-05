auto_counter = 0
def auto(reset=False):
    global auto_counter
    if reset:
        auto_counter = 0
    result = auto_counter
    auto_counter += 1
    return result