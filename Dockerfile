FROM python:3.11

WORKDIR /app

COPY requirements.txt .
RUN pip install -r requirements.txt

COPY shabbat.py .

CMD ["python", "shabbat.py", "--dry-run"]
