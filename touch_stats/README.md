# Install Dependencies

Create a virtual env for tests
```shell
python3 -m virtualenv venv_touch_stats/
```

Activate venv_touch_stats/ virtualenv

```shell
source venv_touch_stats/bin/activate
```

Install dependencies
```shell
pip3 install -r requirements.txt
```

# Run test

Start "Touch Stats" application by tapping it on STAX dashboard

Run the test script and follow instructions on device, test results will be stored in output.csv
```shell
python3 touch_stats.py --fileName output
```

If you want to specify the number of touch points for each test, add the "--nbPoints" argument (if not provided, nbPoints is defaulted to 30)
```shell
python3 touch_stats.py --fileName output --nbPoints 100
```