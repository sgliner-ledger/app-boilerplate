from boilerplate_client.transaction import Transaction
from boilerplate_client.boilerplate_cmd import BoilerplateCommand, Errors
from ragger.backend import RaisePolicy
from utils import create_simple_nav_instructions, ROOT_SCREENSHOT_PATH, unpack_get_public_key_response, unpack_sign_tx_response, check_signature_validity

# In this tests we check the behavior of the device when asked to sign a transaction


# In this test se send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
def test_sign_tx_short_tx(backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommand(backend)
    # The path used for this entire test
    path: str = "m/44'/0'/0'/0/0"

    # First we need to get the public key of the device in order to build the transaction
    rapdu = client.get_public_key(path=path)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    # Create the transaction that will be sent to the device for signing
    transaction = Transaction(
        nonce=1,
        to="0xde0b295669a9fd93d5f28d9ec85e40f4cb697bae",
        value=666,
        memo="For u EthDev"
    ).serialize()

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_tx(path=path, transaction=transaction):
        if backend.firmware.device == "nanos":
            nav_ins = create_simple_nav_instructions(5)
        elif backend.firmware.device.startswith("nano"):
            nav_ins = create_simple_nav_instructions(3)
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH, test_name, nav_ins)

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    assert check_signature_validity(public_key, der_sig, transaction)


# In this test se send to the device a transaction to sign and validate it on screen
# This test is mostly the same as the previous one but with different values.
# In particular the long memo will force the transaction to be sent in multiple chunks
def test_sign_tx_long_tx(backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommand(backend)
    path: str = "m/44'/0'/0'/0/0"

    rapdu = client.get_public_key(path=path)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    transaction = Transaction(
        nonce=1,
        to="0xde0b295669a9fd93d5f28d9ec85e40f4cb697bae",
        value=666,
        memo=("This is a very long memo. "
              "It will force the app client to send the serialized transaction to be sent in chunk. "
              "As the maximum chunk size is 255 bytes we will make this memo greater than 255 characters. "
              "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed non risus. Suspendisse lectus tortor, dignissim sit amet, adipiscing nec, ultricies sed, dolor. Cras elementum ultrices diam.")
    ).serialize()

    with client.sign_tx(path=path, transaction=transaction):
        if backend.firmware.device == "nanos":
            nav_ins = create_simple_nav_instructions(5)
        elif backend.firmware.device.startswith("nano"):
            nav_ins = create_simple_nav_instructions(3)
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH, test_name, nav_ins)

    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    assert check_signature_validity(public_key, der_sig, transaction)



# Transaction signature refused test
# The test will ask for a transaction signature that will be refused on screen

def test_sign_tx_refused(backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommand(backend)
    path: str = "m/44'/0'/0'/0/0"

    rapdu = client.get_public_key(path=path)
    _, pub_key, _, _ = unpack_get_public_key_response(rapdu.data)

    transaction = Transaction(
        nonce=1,
        to="0xde0b295669a9fd93d5f28d9ec85e40f4cb697bae",
        value=666,
        memo="This transaction will be refused by the user"
    ).serialize()

    with client.sign_tx(path=path, transaction=transaction):
        if backend.firmware.device == "nanos":
            nav_ins = create_simple_nav_instructions(5 + 1)
        elif backend.firmware.device.startswith("nano"):
            nav_ins = create_simple_nav_instructions(3 + 1)
        # Disable raising when trying to unpack an error APDU
        backend.raise_policy = RaisePolicy.RAISE_NOTHING
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH, test_name, nav_ins)

    assert client.get_async_response().status == Errors.SW_DENY
