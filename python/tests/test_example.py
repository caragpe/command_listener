from python.examples.example import (
    call_process_command,
    load_return_code_messages,
    get_lib_filename,
)
import pytest


def test_load_return_code_messages_default(mocker):
    """Test loading default error messages when config file is missing."""
    mocker.patch('pathlib.Path.exists', return_value=False)
    messages = load_return_code_messages()
    assert messages[0] == "Success"
    assert messages[-1] == "Invalid buffer or buffer size is zero"


def test_load_return_code_messages_with_config(mocker):
    """Test loading error messages from a config file."""
    mocker.patch('pathlib.Path.exists', return_value=True)
    mock_read = mocker.patch('configparser.ConfigParser.read')
    mocker.patch('configparser.ConfigParser.sections', return_value=['codes'])
    mocker.patch('configparser.ConfigParser.items',
                 return_value=[('0', 'Success'), ('-1', 'Test Error')])

    messages = load_return_code_messages()
    assert messages[0] == "Success"
    assert messages[-1] == "Test Error"

    mock_read.assert_called_once()


def test_get_lib_filename(mocker):
    """Test platform-specific library filename selection."""
    mocker.patch('platform.system', return_value='Linux')
    mocker.patch('platform.machine', return_value='x86_64')
    assert get_lib_filename() == "libprocess_command.so"

    mocker.patch('platform.system', return_value='Windows')
    assert get_lib_filename() == "process_command.dll"

    mocker.patch('platform.system', return_value='Unknown')
    with pytest.raises(ValueError):
        get_lib_filename()


def test_call_process_command_success(mocker):
    """Test successful command execution."""
    mocker.patch('pathlib.Path.exists', return_value=True)
    mock_cdll = mocker.patch('ctypes.CDLL')
    mock_lib = mocker.MagicMock()
    mock_cdll.return_value = mock_lib
    mock_lib.process_command.return_value = 0

    code, resp = call_process_command("test", bufsize=1024, encoding='ascii')
    assert code == 0
    assert resp is not None


def test_call_process_command_truncation_warning(mocker):
    """Test truncation warning for full buffer."""
    mocker.patch('pathlib.Path.exists', return_value=True)
    mock_cdll = mocker.patch('ctypes.CDLL')
    mock_lib = mocker.MagicMock()
    mock_cdll.return_value = mock_lib
    mock_lib.process_command.return_value = 0

    mocker.patch('python.examples.example.decode_bytes', return_value="x" * 1023)
    code, resp = call_process_command("test", bufsize=1024, encoding='ascii')
    assert code == 0
    assert resp == "x" * 1023


def test_call_process_command_invalid_command():
    """Test error for empty command."""
    with pytest.raises(ValueError):
        call_process_command("")


def test_call_process_command_invalid_bufsize():
    """Test error for invalid buffer size."""
    with pytest.raises(ValueError):
        call_process_command("test", bufsize=0)


def test_call_process_command_missing_library(mocker):
    """Test error for missing library file."""
    mocker.patch('pathlib.Path.exists', return_value=False)
    with pytest.raises(FileNotFoundError):
        call_process_command("test")
